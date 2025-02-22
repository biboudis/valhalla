/*
 * Copyright (c) 2019, Red Hat, Inc. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#ifndef SHARE_GC_SHARED_BARRIERSET_INLINE_HPP
#define SHARE_GC_SHARED_BARRIERSET_INLINE_HPP

#include "gc/shared/barrierSet.hpp"

#include "oops/accessDecorators.hpp"
#include "oops/arrayOop.hpp"
#include "oops/compressedOops.inline.hpp"
#include "oops/objArrayOop.inline.hpp"
#include "oops/oop.hpp"
#include "runtime/thread.hpp"

template <DecoratorSet decorators, typename BarrierSetT>
template <typename T>
inline void BarrierSet::AccessBarrier<decorators, BarrierSetT>::oop_arraycopy_in_heap(arrayOop src_obj, size_t src_offset_in_bytes, T* src_raw,
                                                                                      arrayOop dst_obj, size_t dst_offset_in_bytes, T* dst_raw,
                                                                                      size_t length) {
  T* src = arrayOopDesc::obj_offset_to_raw(src_obj, src_offset_in_bytes, src_raw);
  T* dst = arrayOopDesc::obj_offset_to_raw(dst_obj, dst_offset_in_bytes, dst_raw);

  if ((!HasDecorator<decorators, ARRAYCOPY_CHECKCAST>::value) &&
      (!HasDecorator<decorators, ARRAYCOPY_NOTNULL>::value)) {
    // Covariant, copy without checks
    Raw::oop_arraycopy(NULL, 0, src, NULL, 0, dst, length);
    return;
  }

  // Copy each element with checking casts
  Klass* const dst_klass = objArrayOop(dst_obj)->element_klass();
  for (const T* const end = src + length; src < end; src++, dst++) {
    const T elem = *src;
    if (HasDecorator<decorators, ARRAYCOPY_NOTNULL>::value && CompressedOops::is_null(elem)) {
      throw_array_null_pointer_store_exception(src_obj, dst_obj, JavaThread::current());
      return;
    }
    if (HasDecorator<decorators, ARRAYCOPY_CHECKCAST>::value &&
        (!oopDesc::is_instanceof_or_null(CompressedOops::decode(elem), dst_klass))) {
      throw_array_store_exception(src_obj, dst_obj, JavaThread::current());
      return;
    }
    *dst = elem;
  }
}

#endif // SHARE_GC_SHARED_BARRIERSET_INLINE_HPP

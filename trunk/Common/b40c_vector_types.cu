/**
 * Copyright 2010 Duane Merrill
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 * 
 * For more information, see our Google Code project site: 
 * http://code.google.com/p/back40computing/
 * 
 * Thanks!
 */

/******************************************************************************
 * Utility code for working with vector types of arbitary typenames
 ******************************************************************************/

#pragma once

namespace b40c {


/**
 * Specializations of this vector-type template can be used to indicate the 
 * proper vector type for a given typename and vector size. We can use the ::Type
 * typedef to declare and work with the appropriate vectorized type for a given 
 * typename T.
 * 
 * For example, consider the following copy kernel that uses vec-2 loads 
 * and stores:
 * 
 * 		template <typename T>
 * 		__global__ void CopyKernel(T *d_in, T *d_out) 
 * 		{
 * 			typename VecType<T, 2>::Type datum;
 * 
 * 			typename VecType<T, 2>::Type *d_in_v2 = (typename VecType<T, 2>::Type *) d_in;
 * 			typename VecType<T, 2>::Type *d_out_v2 = (typename VecType<T, 2>::Type *) d_out;
 * 
 * 			datum = in[threadIdx.x];
 * 			out[threadIdx.x] = datum;
 * 		} 
 * 
 * We can use this kernel to copy int streams using 64-bit loads, e.g.:  
 * 
 * 		CopyKernel<int><<<1, 128>>>(d_in, d_out) 
 *    
 */
template <typename T, int vec_elements> struct VecType;

/**
 * Partially-specialized generic vec1 type 
 */
template <typename T> 
struct VecType<T, 1> {
	T x;
	typedef T Type;
};

/**
 * Partially-specialized generic vec2 type 
 */
template <typename T> 
struct VecType<T, 2> {
	T x;
	T y;
	typedef VecType<T, 2> Type;
};

/**
 * Partially-specialized generic vec4 type 
 */
template <typename T> 
struct VecType<T, 4> {
	T x;
	T y;
	T z;
	T w;
	typedef VecType<T, 4> Type;
};


/**
 * Macro for expanding partially-specialized built-in vector types
 */
#define B40C_DEFINE_VECTOR_TYPE(base_type,short_type)                           \
  template<> struct VecType<base_type, 1> { typedef short_type##1 Type; };      \
  template<> struct VecType<base_type, 2> { typedef short_type##2 Type; };      \
  template<> struct VecType<base_type, 4> { typedef short_type##4 Type; };     

B40C_DEFINE_VECTOR_TYPE(char,               char)
B40C_DEFINE_VECTOR_TYPE(signed char,        char)
B40C_DEFINE_VECTOR_TYPE(short,              short)
B40C_DEFINE_VECTOR_TYPE(int,                int)
B40C_DEFINE_VECTOR_TYPE(long,               long)
B40C_DEFINE_VECTOR_TYPE(long long,          longlong)
B40C_DEFINE_VECTOR_TYPE(unsigned char,      uchar)
B40C_DEFINE_VECTOR_TYPE(unsigned short,     ushort)
B40C_DEFINE_VECTOR_TYPE(unsigned int,       uint)
B40C_DEFINE_VECTOR_TYPE(unsigned long,      ulong)
B40C_DEFINE_VECTOR_TYPE(unsigned long long, ulonglong)
B40C_DEFINE_VECTOR_TYPE(float,              float)
B40C_DEFINE_VECTOR_TYPE(double,             double)

#undef B40C_DEFINE_VECTOR_TYPE


} // namespace b40c


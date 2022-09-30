/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuchanghui<liuchanghui@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _MD_TYPES
#define _MD_TYPES

#include "cpu.h"

#if __WORDSIZE == 64
  #define ELF_CLASS ELFCLASS64
#else
  #define ELF_CLASS ELFCLASS32
#endif

#if defined(__x86_64__)
  #define ELF_ARCH  EM_X86_64
#elif defined(__i386__)
  #define ELF_ARCH  EM_386
#elif defined(__arm__)
  #define ELF_ARCH  EM_ARM
#elif defined(__mips__)
  #define ELF_ARCH  EM_MIPS
#elif defined(__aarch64__)
  #define ELF_ARCH  EM_AARCH64
#elif defined(__sw_64)
  #define ELF_ARCH  EM_SW_64
#endif

typedef union {
  struct {
    uint32_t vendor_id[3];               /* cpuid 0: ebx, edx, ecx */
    uint32_t version_information;        /* cpuid 1: eax */
    uint32_t feature_information;        /* cpuid 1: edx */
    uint32_t amd_extended_cpu_features;  /* cpuid 0x80000001, ebx */
  } x86_cpu_info;
  struct {
    uint32_t cpuid;
    uint32_t elf_hwcaps;    /* linux specific, 0 otherwise */
  } arm_cpu_info;
  struct {
    uint64_t processor_features[2];
  } other_cpu_info;
} MDCPUInformation;  /* CPU_INFORMATION */

/* For (MDCPUInformation).arm_cpu_info.elf_hwcaps.
 * This matches the Linux kernel definitions from <asm/hwcaps.h> */
typedef enum {
  MD_CPU_ARM_ELF_HWCAP_SWP       = (1 << 0),
  MD_CPU_ARM_ELF_HWCAP_HALF      = (1 << 1),
  MD_CPU_ARM_ELF_HWCAP_THUMB     = (1 << 2),
  MD_CPU_ARM_ELF_HWCAP_26BIT     = (1 << 3),
  MD_CPU_ARM_ELF_HWCAP_FAST_MULT = (1 << 4),
  MD_CPU_ARM_ELF_HWCAP_FPA       = (1 << 5),
  MD_CPU_ARM_ELF_HWCAP_VFP       = (1 << 6),
  MD_CPU_ARM_ELF_HWCAP_EDSP      = (1 << 7),
  MD_CPU_ARM_ELF_HWCAP_JAVA      = (1 << 8),
  MD_CPU_ARM_ELF_HWCAP_IWMMXT    = (1 << 9),
  MD_CPU_ARM_ELF_HWCAP_CRUNCH    = (1 << 10),
  MD_CPU_ARM_ELF_HWCAP_THUMBEE   = (1 << 11),
  MD_CPU_ARM_ELF_HWCAP_NEON      = (1 << 12),
  MD_CPU_ARM_ELF_HWCAP_VFPv3     = (1 << 13),
  MD_CPU_ARM_ELF_HWCAP_VFPv3D16  = (1 << 14),
  MD_CPU_ARM_ELF_HWCAP_TLS       = (1 << 15),
  MD_CPU_ARM_ELF_HWCAP_VFPv4     = (1 << 16),
  MD_CPU_ARM_ELF_HWCAP_IDIVA     = (1 << 17),
  MD_CPU_ARM_ELF_HWCAP_IDIVT     = (1 << 18),
} MDCPUInformationARMElfHwCaps;

/* For (MDRawSystemInfo).processor_architecture: */
typedef enum {
  MD_CPU_ARCHITECTURE_X86       =  0,  /* PROCESSOR_ARCHITECTURE_INTEL */
  MD_CPU_ARCHITECTURE_MIPS      =  1,  /* PROCESSOR_ARCHITECTURE_MIPS */
  MD_CPU_ARCHITECTURE_ALPHA     =  2,  /* PROCESSOR_ARCHITECTURE_ALPHA */
  MD_CPU_ARCHITECTURE_PPC       =  3,  /* PROCESSOR_ARCHITECTURE_PPC */
  MD_CPU_ARCHITECTURE_SHX       =  4,  /* PROCESSOR_ARCHITECTURE_SHX
                                        * (Super-H) */
  MD_CPU_ARCHITECTURE_ARM       =  5,  /* PROCESSOR_ARCHITECTURE_ARM */
  MD_CPU_ARCHITECTURE_IA64      =  6,  /* PROCESSOR_ARCHITECTURE_IA64 */
  MD_CPU_ARCHITECTURE_ALPHA64   =  7,  /* PROCESSOR_ARCHITECTURE_ALPHA64 */
  MD_CPU_ARCHITECTURE_MSIL      =  8,  /* PROCESSOR_ARCHITECTURE_MSIL
                                        * (Microsoft Intermediate Language) */
  MD_CPU_ARCHITECTURE_AMD64     =  9,  /* PROCESSOR_ARCHITECTURE_AMD64 */
  MD_CPU_ARCHITECTURE_X86_WIN64 = 10,  /* PROCESSOR_ARCHITECTURE_IA32_ON_WIN64 (WoW64) */
  MD_CPU_ARCHITECTURE_SPARC     = 0x8001, /* Breakpad-defined value for SPARC */
  MD_CPU_ARCHITECTURE_PPC64     = 0x8002, /* Breakpad-defined value for PPC64 */
  MD_CPU_ARCHITECTURE_ARM64     = 0x8003, /* Breakpad-defined value for ARM64 */
  MD_CPU_ARCHITECTURE_MIPS64    = 0x8004, /* Breakpad-defined value for MIPS64 */
  MD_CPU_ARCHITECTURE_SW64      = 0x8005, /* Breakpad-defined value for SunWay 64 */
  MD_CPU_ARCHITECTURE_UNKNOWN   = 0xffff  /* PROCESSOR_ARCHITECTURE_UNKNOWN */
} MDCPUArchitecture;


typedef uint32_t MDRVA;  /* RVA */

const MDRVA kInvalidMDRVA = static_cast<MDRVA>(-1);

typedef struct {
  char             mode[4]; /*record mode:fast or hard*/
  /* The next 3 fields and numberOfProcessors are from the SYSTEM_INFO
   * structure as returned by GetSystemInfo */
  uint16_t         processor_architecture;
  uint16_t         processor_level;         /* x86: 5 = 586, 6 = 686, ... */
                                            /* ARM: 6 = ARMv6, 7 = ARMv7 ... */
  uint16_t         processor_revision;      /* x86: 0xMMSS, where MM=model,
                                             *      SS=stepping */
                                            /* ARM: 0 */

  uint16_t         number_of_processors;
  uint8_t          uname[128];
  uint8_t          lsb_release[128];

  MDCPUInformation cpu;
} MDRawSystemInfo;  /* MINIDUMP_SYSTEM_INFO */

#define MD_EXCEPTION_MAXIMUM_PARAMETERS 15

typedef struct {
  uint32_t  exception_code;     /* Windows: MDExceptionCodeWin,
                                 * Mac OS X: MDExceptionMac,
                                 * Linux: MDExceptionCodeLinux. */
  uint32_t  exception_flags;    /* Windows: 1 if noncontinuable,
                                   Mac OS X: MDExceptionCodeMac. */
  uint64_t  exception_record;   /* Address (in the minidump-producing host's
                                 * memory) of another MDException, for
                                 * nested exceptions. */
  uint64_t  exception_address;  /* The address that caused the exception.
                                 * Mac OS X: exception subcode (which is
                                 *           typically the address). */
  uint32_t  number_parameters;  /* Number of valid elements in
                                 * exception_information. */
  uint32_t  __align;
  uint64_t  exception_information[MD_EXCEPTION_MAXIMUM_PARAMETERS];
} MDException;  /* MINIDUMP_EXCEPTION */

typedef struct {
  uint32_t             thread_id;         /* Thread in which the exception
                                           * occurred.  Corresponds to
                                           * (MDRawThread).thread_id. */
  uint32_t             __align;
  MDException          exception_record;
} MDRawExceptionStream;  /* MINIDUMP_EXCEPTION_STREAM */

const int kMDGUIDSize = 20;

typedef struct {
  uint64_t             base_of_image;
  uint32_t             size_of_image;
  uint32_t             checksum;         /* 0 if unknown */
  uint32_t             time_date_stamp;  /* time_t */
  uint8_t              guid[kMDGUIDSize];
  char                 file_path[NAME_MAX];  /* MDString, pathname or filename */
} MDRawModule;  /* MINIDUMP_MODULE */

/* These structs are used to store the DSO debug data in Linux minidumps,
 * which is necessary for converting minidumps to usable coredumps.
 * Because of a historical accident, several fields are variably encoded
 * according to client word size, so tools potentially need to support both. */

typedef struct {
  uint32_t  addr;
  uint32_t  ld;
  char     name[NAME_MAX];
} MDRawLinkMap32;

typedef struct {
  uint32_t  version;
  uint32_t  dso_count;
  uint32_t  brk;
  uint32_t  ldbase;
  uint32_t  dynamic;
} MDRawDebug32;

typedef struct {
  uint64_t  addr;
  uint64_t  ld;
  char     name[NAME_MAX];
} MDRawLinkMap64;

typedef struct {
  uint32_t  version;
  uint32_t  dso_count;
  uint64_t  brk;
  uint64_t  ldbase;
  uint64_t  dynamic;
#if defined(__mips64)
  uint64_t  rld_map;
#endif
} MDRawDebug64;

template <size_t>
struct MDTypeHelper;

template <>
struct MDTypeHelper<sizeof(uint32_t)> {
  typedef MDRawDebug32 MDRawDebug;
  typedef MDRawLinkMap32 MDRawLinkMap;
};

template <>
struct MDTypeHelper<sizeof(uint64_t)> {
  typedef MDRawDebug64 MDRawDebug;
  typedef MDRawLinkMap64 MDRawLinkMap;
};

typedef MDTypeHelper<sizeof(ElfW(Addr))>::MDRawDebug MDRawDebug;
typedef MDTypeHelper<sizeof(ElfW(Addr))>::MDRawLinkMap MDRawLinkMap;


///////////////////////////////////////////////////////////////////////
//
// A lightweight wrapper with a pointer and a length to encapsulate a
// contiguous range of memory. It provides helper methods for checked
// access of a subrange of the memory. Its implemementation does not
// allocate memory or call into libc functions, and is thus safer to use
// in a crashed environment.
class MemoryRange {
 public:
  MemoryRange() : data_(nullptr), length_(0) {}
  MemoryRange(const std::string& str) { Set(str.data(), str.length()); }

  MemoryRange(const void* data, size_t length) {
    Set(data, length);
  }

  // Returns true if this memory range contains no data.
  bool IsEmpty() const {
    // Set() guarantees that |length_| is zero if |data_| is nullptr.
    return length_ == 0;
  }

  // Resets to an empty range.
  void Reset() {
    data_ = nullptr;
    length_ = 0;
  }

  // Sets this memory range to point to |data| and its length to |length|.
  void Set(const void* data, size_t length) {
    data_ = reinterpret_cast<const uint8_t*>(data);
    // Always set |length_| to zero if |data_| is nullptr.
    length_ = data ? length : 0;
  }

  // Returns true if this range covers a subrange of |sub_length| bytes
  // at |sub_offset| bytes of this memory range, or false otherwise.
  bool Covers(size_t sub_offset, size_t sub_length) const {
    // The following checks verify that:
    // 1. sub_offset is within [ 0 .. length_ - 1 ]
    // 2. sub_offset + sub_length is within
    //    [ sub_offset .. length_ ]
    return sub_offset < length_ &&
           sub_offset + sub_length >= sub_offset &&
           sub_offset + sub_length <= length_;
  }

  // Returns a raw data pointer to a subrange of |sub_length| bytes at
  // |sub_offset| bytes of this memory range, or nullptr if the subrange
  // is out of bounds.
  const void* GetData(size_t sub_offset, size_t sub_length) const {
    return Covers(sub_offset, sub_length) ? (data_ + sub_offset) : nullptr;
  }

  // Same as the two-argument version of GetData() but uses sizeof(DataType)
  // as the subrange length and returns an |DataType| pointer for convenience.
  template <typename DataType>
  const DataType* GetData(size_t sub_offset) const {
    return reinterpret_cast<const DataType*>(
        GetData(sub_offset, sizeof(DataType)));
  }

  // Returns a raw pointer to the |element_index|-th element of an array
  // of elements of length |element_size| starting at |sub_offset| bytes
  // of this memory range, or nullptr if the element is out of bounds.
  const void* GetArrayElement(size_t element_offset,
                              size_t element_size,
                              unsigned element_index) const {
    size_t sub_offset = element_offset + element_index * element_size;
    return GetData(sub_offset, element_size);
  }

  // Same as the three-argument version of GetArrayElement() but deduces
  // the element size using sizeof(ElementType) and returns an |ElementType|
  // pointer for convenience.
  template <typename ElementType>
  const ElementType* GetArrayElement(size_t element_offset,
                                     unsigned element_index) const {
    return reinterpret_cast<const ElementType*>(
        GetArrayElement(element_offset, sizeof(ElementType), element_index));
  }

  // Returns a subrange of |sub_length| bytes at |sub_offset| bytes of
  // this memory range, or an empty range if the subrange is out of bounds.
  MemoryRange Subrange(size_t sub_offset, size_t sub_length) const {
    return Covers(sub_offset, sub_length) ?
        MemoryRange(data_ + sub_offset, sub_length) : MemoryRange();
  }

  // Returns a pointer to the beginning of this memory range.
  const uint8_t* data() const { return data_; }

  // Returns the length, in bytes, of this memory range.
  size_t length() const { return length_; }

 private:
  // Pointer to the beginning of this memory range.
  const uint8_t* data_;

  // Length, in bytes, of this memory range.
  size_t length_;
};

// Typedef for our parsing of the auxv variables in /proc/pid/auxv.
#if defined(__i386) || defined(__ARM_EABI__) || \
 (defined(__mips__) && _MIPS_SIM == _ABIO32)
typedef Elf32_auxv_t elf_aux_entry;
#elif defined(__x86_64) || defined(__aarch64__) || defined(__sw_64) || \
     (defined(__mips__) && _MIPS_SIM != _ABIO32)
typedef Elf64_auxv_t elf_aux_entry;
#endif

typedef __typeof__(((elf_aux_entry*) 0)->a_un.a_val) elf_aux_val_t;


#endif /*end #ifndef _MD_TYPES*/

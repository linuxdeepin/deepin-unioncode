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
static const char kLinuxGateLibraryName[] = "linux-gate.so";
static const char kMappedFileUnsafePrefix[] = "/dev/";
static const char kDeletedSuffix[] = " (deleted)";

// Traits classes so consumers can write templatized code to deal
// with specific ELF bits.
struct ElfClass32 {
  typedef Elf32_Addr Addr;
  typedef Elf32_Ehdr Ehdr;
  typedef Elf32_Nhdr Nhdr;
  typedef Elf32_Phdr Phdr;
  typedef Elf32_Shdr Shdr;
  typedef Elf32_Half Half;
  typedef Elf32_Off Off;
  typedef Elf32_Sym Sym;
  typedef Elf32_Word Word;
  typedef struct {
    int32_t d_tag;
    uint32_t d_val;
  } ElfDyn;

  static const int kClass = ELFCLASS32;
  static const uint16_t kMachine = EM_386;
  static const size_t kAddrSize = sizeof(Elf32_Addr);
  static constexpr const char* kMachineName = "x86";
};

struct ElfClass64 {
  typedef Elf64_Addr Addr;
  typedef Elf64_Ehdr Ehdr;
  typedef Elf64_Nhdr Nhdr;
  typedef Elf64_Phdr Phdr;
  typedef Elf64_Shdr Shdr;
  typedef Elf64_Half Half;
  typedef Elf64_Off Off;
  typedef Elf64_Sym Sym;
  typedef Elf64_Word Word;
  typedef struct {
    uint64_t d_tag;
    uint64_t d_val;
  } ElfDyn;

  static const int kClass = ELFCLASS64;
  static const uint16_t kMachine = EM_X86_64;
  static const size_t kAddrSize = sizeof(Elf64_Addr);
  static constexpr const char* kMachineName = "x86_64";
};

struct ElfSegment {
  const void* start;
  size_t size;
};

#define	ELFMAG		"\177ELF"
#define	SELFMAG		4
bool IsValidElf(const void* elf_base)
{
  return strncmp(reinterpret_cast<const char*>(elf_base),
                    ELFMAG, SELFMAG) == 0;
}

int ElfClass(const void* elf_base)
{
  const ElfW(Ehdr)* elf_header =
    reinterpret_cast<const ElfW(Ehdr)*>(elf_base);

  return elf_header->e_ident[EI_CLASS];
}

template<typename ElfClass, typename T>
const T* GetOffset(const typename ElfClass::Ehdr* elf_header,
                   typename ElfClass::Off offset) {
  return reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(elf_header) +
                                    offset);
}

template<typename ElfClass>
void FindElfClassSegment(const char *elf_base,
                         typename ElfClass::Word segment_type,
                         vector<ElfSegment> *segments)
{
  typedef typename ElfClass::Ehdr Ehdr;
  typedef typename ElfClass::Phdr Phdr;

  assert(elf_base);
  assert(segments);

  assert(strncmp(elf_base, ELFMAG, SELFMAG) == 0);

  const Ehdr* elf_header = reinterpret_cast<const Ehdr*>(elf_base);
  assert(elf_header->e_ident[EI_CLASS] == ElfClass::kClass);

  const Phdr* phdrs =
    GetOffset<ElfClass, Phdr>(elf_header, elf_header->e_phoff);

  for (int i = 0; i < elf_header->e_phnum; ++i) {
    if (phdrs[i].p_type == segment_type) {
      ElfSegment seg = {0,0};
      seg.start = elf_base + phdrs[i].p_offset;
      seg.size = phdrs[i].p_filesz;
      segments->push_back(seg);
    }
  }
}

template<typename ElfClass>
const typename ElfClass::Shdr* FindElfSectionByName(
    const char* name,
    typename ElfClass::Word section_type,
    const typename ElfClass::Shdr* sections,
    const char* section_names,
    const char* names_end,
    int nsection)
{
  assert(name != NULL);
  assert(sections != NULL);
  assert(nsection > 0);

  int name_len = strlen(name);
  if (name_len == 0)
    return NULL;

  for (int i = 0; i < nsection; ++i) {
    const char* section_name = section_names + sections[i].sh_name;
    if (sections[i].sh_type == section_type &&
        names_end - section_name >= name_len + 1 &&
        strcmp(name, section_name) == 0) {
      return sections + i;
    }
  }
  return NULL;
}

template<typename ElfClass>
void FindElfClassSection(const char *elf_base,
                         const char *section_name,
                         typename ElfClass::Word section_type,
                         const void **section_start,
                         size_t *section_size) {
  typedef typename ElfClass::Ehdr Ehdr;
  typedef typename ElfClass::Shdr Shdr;

  assert(elf_base);
  assert(section_start);
  assert(section_size);

  assert(strncmp(elf_base, ELFMAG, SELFMAG) == 0);

  const Ehdr* elf_header = reinterpret_cast<const Ehdr*>(elf_base);
  assert(elf_header->e_ident[EI_CLASS] == ElfClass::kClass);

  const Shdr* sections =
    GetOffset<ElfClass, Shdr>(elf_header, elf_header->e_shoff);
  const Shdr* section_names = sections + elf_header->e_shstrndx;
  const char* names =
    GetOffset<ElfClass, char>(elf_header, section_names->sh_offset);
  const char *names_end = names + section_names->sh_size;

  const Shdr* section =
    FindElfSectionByName<ElfClass>(section_name, section_type,
                                   sections, names, names_end,
                                   elf_header->e_shnum);

  if (section != NULL && section->sh_size > 0) {
    *section_start = elf_base + section->sh_offset;
    *section_size = section->sh_size;
  }
}

template<typename ElfClass>
const char* FindLibClassStartWith(const void *elf_base,
                         size_t size, const char* prefix)
{
    typedef typename ElfClass::Shdr Shdr;
    typedef typename ElfClass::ElfDyn ElfDyn;

    assert(elf_base);

    Shdr* dynamic = nullptr;
    size_t dynamic_size = 0;
    FindElfClassSection<ElfClass>((const char*)elf_base, ".dynamic",
                SHT_PROGBITS, (const void**)&dynamic, &dynamic_size);
    if (nullptr == dynamic) {
        return nullptr;
    }

    Shdr* dynstr = nullptr;
    size_t dynstr_size = 0;
    FindElfClassSection<ElfClass>((const char*)elf_base, ".dynstr",
                SHT_PROGBITS, (const void**)&dynstr, &dynstr_size);
    if (nullptr == dynstr) {
        return nullptr;
    }

    if (dynamic->sh_entsize != sizeof(ElfDyn)) {
        LOG(DEBUG) << "Invalid ELF file: incorrect .dynamic size "
               << dynamic->sh_entsize;
        return nullptr;
    }
    if (!dynamic->sh_size) {
        return nullptr;
    }
    if (dynamic->sh_size % dynamic->sh_entsize) {
        LOG(DEBUG) << "Invalid ELF file: incorrect .dynamic section size "
               << dynamic->sh_size;
        return nullptr;
    }
    if (dynstr->sh_size == 0) {
        LOG(DEBUG) << "Invalid ELF file: empty string table";
        return nullptr;
    }

    if (dynamic->sh_offset + dynamic->sh_size >= size) {
        LOG(DEBUG) << "Invalid ELF file: can't read .dynamic";
        return nullptr;
    }

    ElfDyn* dyn_list = (ElfDyn*)((char *)elf_base + dynamic->sh_offset);
    const char* strtable = (const char*)((char *)elf_base + dynstr->sh_offset);

    for (size_t i = 0; i < dynamic->sh_size / dynamic->sh_entsize; ++i) {
        if (dyn_list[i].d_tag == DT_NEEDED &&
            dyn_list[i].d_val < dynstr->sh_size) {
            const char* name = strtable + dyn_list[i].d_val;
            if (!strcmp(name, prefix)) {
                return name;
            }
        }
    }

    return nullptr;
}

bool FindElfSegments(const void* elf_mapped_base,
                     uint32_t segment_type,
                     vector<ElfSegment>* segments)
{
  assert(elf_mapped_base);
  assert(segments);

  if (!IsValidElf(elf_mapped_base))
    return false;

  int cls = ElfClass(elf_mapped_base);
  const char* elf_base =
    static_cast<const char*>(elf_mapped_base);

  if (cls == ELFCLASS32) {
    FindElfClassSegment<ElfClass32>(elf_base, segment_type, segments);
    return true;
  } else if (cls == ELFCLASS64) {
    FindElfClassSegment<ElfClass64>(elf_base, segment_type, segments);
    return true;
  }

  return false;
}

// ELF note name and desc are 32-bits word padded.
#define NOTE_PADDING(a) ((a + 3) & ~3)

// These functions are also used inside the crashed process, so be safe
// and use the syscall/libc wrappers instead of direct syscalls or libc.

static bool ElfClassBuildIDNoteIdentifier(const void *section, size_t length,
                                          uint8_t* identifier) {
  static_assert(sizeof(ElfClass32::Nhdr) == sizeof(ElfClass64::Nhdr),
                "Elf32_Nhdr and Elf64_Nhdr should be the same");
  typedef typename ElfClass32::Nhdr Nhdr;

  const void* section_end = reinterpret_cast<const char*>(section) + length;
  const Nhdr* note_header = reinterpret_cast<const Nhdr*>(section);
  while (reinterpret_cast<const void *>(note_header) < section_end) {
    if (note_header->n_type == NT_GNU_BUILD_ID)
      break;
    note_header = reinterpret_cast<const Nhdr*>(
                  reinterpret_cast<const char*>(note_header) + sizeof(Nhdr) +
                  NOTE_PADDING(note_header->n_namesz) +
                  NOTE_PADDING(note_header->n_descsz));
  }
  if (reinterpret_cast<const void *>(note_header) >= section_end ||
      note_header->n_descsz == 0) {
    return false;
  }

  const uint8_t* build_id = reinterpret_cast<const uint8_t*>(note_header) +
    sizeof(Nhdr) + NOTE_PADDING(note_header->n_namesz);
  memcpy(identifier, build_id, note_header->n_descsz);

  return true;
}

bool FindElfSection(const void *elf_mapped_base,
                    const char *section_name,
                    uint32_t section_type,
                    const void **section_start,
                    size_t *section_size)
{
  assert(elf_mapped_base);
  assert(section_start);
  assert(section_size);

  *section_start = NULL;
  *section_size = 0;

  if (!IsValidElf(elf_mapped_base))
    return false;

  int cls = ElfClass(elf_mapped_base);
  const char* elf_base =
    static_cast<const char*>(elf_mapped_base);

  if (cls == ELFCLASS32) {
    FindElfClassSection<ElfClass32>(elf_base, section_name, section_type,
                                    section_start, section_size);
    return *section_start != NULL;
  } else if (cls == ELFCLASS64) {
    FindElfClassSection<ElfClass64>(elf_base, section_name, section_type,
                                    section_start, section_size);
    return *section_start != NULL;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
//
//
// A utility class for mapping a file into memory for read-only access of
// the file content. Its implementation avoids calling into libc functions
// by directly making system calls for open, close, mmap, and munmap.
class MemoryMappedFile {
 public:
  MemoryMappedFile();

  // Constructor that calls Map() to map a file at |path| into memory.
  // If Map() fails, the object behaves as if it is default constructed.
  MemoryMappedFile(const char* path, size_t offset);

  ~MemoryMappedFile();

  // Maps a file at |path| into memory, which can then be accessed via
  // content() as a MemoryRange object or via data(), and returns true on
  // success. Mapping an empty file will succeed but with data() and size()
  // returning nullptr and 0, respectively. An existing mapping is unmapped
  // before a new mapping is created.
  bool Map(const char* path, size_t offset);

  // Unmaps the memory for the mapped file. It's a no-op if no file is
  // mapped.
  void Unmap();

  // Returns a MemoryRange object that covers the memory for the mapped
  // file. The MemoryRange object is empty if no file is mapped.
  const MemoryRange& content() const { return content_; }

  // Returns a pointer to the beginning of the memory for the mapped file.
  // or nullptr if no file is mapped or the mapped file is empty.
  const void* data() const { return content_.data(); }

  // Returns the size in bytes of the mapped file, or zero if no file
  // is mapped.
  size_t size() const { return content_.length(); }

 private:
  // Mapped file content as a MemoryRange object.
  MemoryRange content_;
};

MemoryMappedFile::MemoryMappedFile() {}

MemoryMappedFile::MemoryMappedFile(const char* path, size_t offset) {
  Map(path, offset);
}

MemoryMappedFile::~MemoryMappedFile() {
  Unmap();
}

bool MemoryMappedFile::Map(const char* path, size_t offset) {
  Unmap();

  int fd = open(path, O_RDONLY, 0);
  if (fd == -1) {
    return false;
  }

#if defined(__x86_64__) || defined(__aarch64__) || defined(__sw_64) || \
   (defined(__mips__) && _MIPS_SIM == _ABI64)

  struct stat st;
  if (fstat(fd, &st) == -1 || st.st_size < 0) {
#else
  struct kernel_stat64 st;
  if (fstat64(fd, &st) == -1 || st.st_size < 0) {
#endif
    close(fd);
    return false;
  }

  // Strangely file size can be negative, but we check above that it is not.
  size_t file_len = static_cast<size_t>(st.st_size);
  if (0 == file_len) {
    void* data = mmap(nullptr, 16*1024,
            PROT_READ|PROT_WRITE,
            MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return false;
    }

    file_len = read(fd, data, 16*1024);
    close(fd);
    content_.Set(data, file_len);
    return true;
  }

  // If the file does not extend beyond the offset, simply use an empty
  // MemoryRange and return true. Don't bother to call mmap()
  // even though mmap() can handle an empty file on some platforms.
  if (offset >= file_len) {
    close(fd);
    return true;
  }

  void* data = mmap(nullptr, file_len, PROT_READ, MAP_PRIVATE, fd, offset);
  close(fd);
  if (data == MAP_FAILED) {
    return false;
  }

  content_.Set(data, file_len - offset);
  return true;
}

void MemoryMappedFile::Unmap() {
  if (content_.data()) {
    munmap(const_cast<uint8_t*>(content_.data()), content_.length());
    content_.Set(nullptr, 0);
  }
}

// A class for reading a file, line by line, without using fopen/fgets or other
// functions which may allocate memory.
class LineReader {
 public:
  LineReader(int fd)
      : fd_(fd),
        hit_eof_(false),
        buf_used_(0) {
  }

  // The maximum length of a line.
  static const size_t kMaxLineLen = 1024*4;

  // Return the next line from the file.
  //   line: (output) a pointer to the start of the line. The line is NUL
  //     terminated.
  //   len: (output) the length of the line (not inc the NUL byte)
  //
  // Returns true iff successful (false on EOF).
  //
  // One must call |PopLine| after this function, otherwise you'll continue to
  // get the same line over and over.
  bool GetNextLine(const char **line, unsigned int *len) {
    for (;;) {
      if (buf_used_ == 0 && hit_eof_)
        return false;

      for (unsigned int i = 0; i < buf_used_; ++i) {
        if (buf_[i] == '\n') {
          buf_[i] = 0;
          *len = i;
          *line = buf_;
          return true;
        }
      }

      if (buf_used_ == sizeof(buf_)) {
        // we scanned the whole buffer and didn't find an end-of-line marker.
        // This line is too long to process.
        return false;
      }

      // We didn't find any end-of-line terminators in the buffer. However, if
      // this is the last line in the file it might not have one:
      if (hit_eof_) {
        assert(buf_used_);
        // There's room for the NUL because of the buf_used_ == sizeof(buf_)
        // check above.
        buf_[buf_used_] = 0;
        *len = buf_used_;
        buf_used_ += 1;  // since we appended the NUL.
        *line = buf_;
        return true;
      }

      // Otherwise, we should pull in more data from the file
      const ssize_t n = read(fd_, buf_ + buf_used_,
                                 sizeof(buf_) - buf_used_);
      if (n < 0) {
        return false;
      } else if (n == 0) {
        hit_eof_ = true;
      } else {
        buf_used_ += n;
      }

      // At this point, we have either set the hit_eof_ flag, or we have more
      // data to process...
    }
  }

  void PopLine(unsigned int len) {
    // len doesn't include the NUL byte at the end.

    assert(buf_used_ >= len + 1);
    buf_used_ -= len + 1;
    memmove(buf_, buf_ + len + 1, buf_used_);
  }

 private:
  const int fd_;

  bool hit_eof_;
  unsigned int buf_used_;
  char buf_[kMaxLineLen];
};

//===---- lib/ReaderWriter/MachO/SectCreateFile.h ---------------*- c++ -*-===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLD_READER_WRITER_MACHO_SECTCREATE_FILE_H
#define LLD_READER_WRITER_MACHO_SECTCREATE_FILE_H

#include "lld/Core/DefinedAtom.h"
#include "lld/Core/Simple.h"
#include "lld/ReaderWriter/MachOLinkingContext.h"

namespace lld {
namespace mach_o {

//
// A FlateNamespaceFile instance may be added as a resolution source of last
// resort, depending on how -flat_namespace and -undefined are set.
//
class SectCreateFile : public File {
public:

  class SectCreateAtom : public SimpleDefinedAtom {
  public:
    SectCreateAtom(const File &file, StringRef segName, StringRef sectName,
                   std::unique_ptr<MemoryBuffer> content)
      : SimpleDefinedAtom(file),
        _combinedName((segName + "/" + sectName).str()),
        _content(std::move(content)) {}

    uint64_t size() const override { return _content->getBufferSize(); }

    Scope scope() const override { return scopeGlobal; }

    ContentType contentType() const override { return typeSectCreate; }

    SectionChoice sectionChoice() const override { return sectionCustomRequired; }

    StringRef customSectionName() const override { return _combinedName; }

    DeadStripKind deadStrip() const override { return deadStripNever; }

    ArrayRef<uint8_t> rawContent() const override {
      const uint8_t *data =
        reinterpret_cast<const uint8_t*>(_content->getBufferStart());
      return ArrayRef<uint8_t>(data, _content->getBufferSize());
    }

    StringRef segmentName() const { return _segName; }
    StringRef sectionName() const { return _sectName; }

  private:
    std::string _combinedName;
    StringRef _segName;
    StringRef _sectName;
    std::unique_ptr<MemoryBuffer> _content;
  };

  SectCreateFile() : File("sectcreate", kindObject) {}

  void addSection(StringRef seg, StringRef sect,
                  std::unique_ptr<MemoryBuffer> content) {
    _definedAtoms.push_back(
      new (allocator()) SectCreateAtom(*this, seg, sect, std::move(content)));
  }

  const AtomVector<DefinedAtom> &defined() const {
    return _definedAtoms;
  }

  const AtomVector<UndefinedAtom> &undefined() const {
    return _noUndefinedAtoms;
  }

  const AtomVector<SharedLibraryAtom> &sharedLibrary() const {
    return _noSharedLibraryAtoms;
  }

  const AtomVector<AbsoluteAtom> &absolute() const {
    return _noAbsoluteAtoms;
  }

private:
  AtomVector<DefinedAtom> _definedAtoms;
};

} // namespace mach_o
} // namespace lld

#endif // LLD_READER_WRITER_MACHO_SECTCREATE_FILE_H
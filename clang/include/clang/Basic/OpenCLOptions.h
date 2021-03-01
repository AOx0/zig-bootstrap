//===--- OpenCLOptions.h ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the clang::OpenCLOptions class.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_OPENCLOPTIONS_H
#define LLVM_CLANG_BASIC_OPENCLOPTIONS_H

#include "clang/Basic/LangOptions.h"
#include "llvm/ADT/StringMap.h"

namespace clang {

namespace {
// This enum maps OpenCL version(s) into value. These values are used as
// a mask to indicate in which OpenCL version(s) extension is a core or
// optional core feature.
enum OpenCLVersionID : unsigned int {
  OCL_C_10 = 0x1,
  OCL_C_11 = 0x2,
  OCL_C_12 = 0x4,
  OCL_C_20 = 0x8,
  OCL_C_30 = 0x10,
  OCL_C_ALL = 0x1f,
  OCL_C_11P = OCL_C_ALL ^ OCL_C_10,              // OpenCL C 1.1+
  OCL_C_12P = OCL_C_ALL ^ (OCL_C_10 | OCL_C_11), // OpenCL C 1.2+
};

static inline OpenCLVersionID encodeOpenCLVersion(unsigned OpenCLVersion) {
  switch (OpenCLVersion) {
  default:
    llvm_unreachable("Unknown OpenCL version code");
  case 100:
    return OCL_C_10;
  case 110:
    return OCL_C_11;
  case 120:
    return OCL_C_12;
  case 200:
    return OCL_C_20;
  case 300:
    return OCL_C_30;
  }
}

// Simple helper to check if OpenCL C version is contained in a given encoded
// OpenCL C version mask
static inline bool isOpenCLVersionIsContainedInMask(const LangOptions &LO,
                                                    unsigned Mask) {
  auto CLVer = LO.OpenCLCPlusPlus ? 200 : LO.OpenCLVersion;
  OpenCLVersionID Code = encodeOpenCLVersion(CLVer);
  return Mask & Code;
}
} // end anonymous namespace

/// OpenCL supported extensions and optional core features
class OpenCLOptions {
public:
  struct OpenCLOptionInfo {
    // Option starts to be available in this OpenCL version
    unsigned Avail;

    // Option becomes core feature in this OpenCL versions
    unsigned Core;

    // Option becomes optional core feature in this OpenCL versions
    unsigned Opt;

    // Is this option supported
    bool Supported = false;

    // Is this option enabled
    bool Enabled = false;

    OpenCLOptionInfo(unsigned A = 100, unsigned C = 0U, unsigned O = 0U)
        : Avail(A), Core(C), Opt(O) {}

    bool isCore() const { return Core != 0U; }

    bool isOptionalCore() const { return Opt != 0U; }

    // Is option available in OpenCL version \p LO.
    bool isAvailableIn(const LangOptions &LO) const {
      // In C++ mode all extensions should work at least as in v2.0.
      auto CLVer = LO.OpenCLCPlusPlus ? 200 : LO.OpenCLVersion;
      return CLVer >= Avail;
    }

    // Is core option in OpenCL version \p LO.
    bool isCoreIn(const LangOptions &LO) const {
      return isAvailableIn(LO) && isOpenCLVersionIsContainedInMask(LO, Core);
    }

    // Is optional core option in OpenCL version \p LO.
    bool isOptionalCoreIn(const LangOptions &LO) const {
      return isAvailableIn(LO) && isOpenCLVersionIsContainedInMask(LO, Opt);
    }
  };

  bool isKnown(llvm::StringRef Ext) const;

  bool isEnabled(llvm::StringRef Ext) const;

  // Is supported as either an extension or an (optional) core feature for
  // OpenCL version \p LO.
  bool isSupported(llvm::StringRef Ext, const LangOptions &LO) const;

  // Is supported OpenCL core feature for OpenCL version \p LO.
  // For supported extension, return false.
  bool isSupportedCore(llvm::StringRef Ext, const LangOptions &LO) const;

  // Is supported optional core OpenCL feature for OpenCL version \p LO.
  // For supported extension, return false.
  bool isSupportedOptionalCore(llvm::StringRef Ext,
                               const LangOptions &LO) const;

  // Is supported optional core or core OpenCL feature for OpenCL version \p
  // LO. For supported extension, return false.
  bool isSupportedCoreOrOptionalCore(llvm::StringRef Ext,
                                     const LangOptions &LO) const;

  // Is supported OpenCL extension for OpenCL version \p LO.
  // For supported core or optional core feature, return false.
  bool isSupportedExtension(llvm::StringRef Ext, const LangOptions &LO) const;

  void enable(llvm::StringRef Ext, bool V = true);

  /// Enable or disable support for OpenCL extensions
  /// \param Ext name of the extension (not prefixed with '+' or '-')
  /// \param V value to set for a extension
  void support(llvm::StringRef Ext, bool V = true);

  OpenCLOptions();
  OpenCLOptions(const OpenCLOptions &) = default;

  // Set supported options based on target settings and language version
  void addSupport(const llvm::StringMap<bool> &FeaturesMap,
                  const LangOptions &Opts);

  // Disable all extensions
  void disableAll();

  // Enable supported core and optional core features
  void enableSupportedCore(const LangOptions &LO);

  friend class ASTWriter;
  friend class ASTReader;

  using OpenCLOptionInfoMap = llvm::StringMap<OpenCLOptionInfo>;

private:
  OpenCLOptionInfoMap OptMap;
};

} // end namespace clang

#endif

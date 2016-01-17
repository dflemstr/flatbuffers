/*
 * Copyright 2014 Google Inc. All rights reserved.
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
 */

// independent from idl_parser, since this code is not needed for most clients

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {
namespace rust {

static std::string GenTypeBasic(const Type &type) {
  static const char *rusttypename[] = {
#define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, RTYPE) \
    #RTYPE,
    FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
#undef FLATBUFFERS_TD
  };
  return rusttypename[type.base_type];
}

static std::string GenType(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING:
      return "::flatbuffers::FlatStr<'a>";
    case BASE_TYPE_VECTOR:
      return "::flatbuffers::FlatVec<'a, " + GenType(type.VectorType()) + ">";
    case BASE_TYPE_STRUCT:
      return type.struct_def->name;
    default:
      break;
  }

  if (IsScalar(type.base_type)) {
    return GenTypeBasic(type);
  } else {
    return "()";
  }
}

static void GenEnum(EnumDef &enum_def, std::string &code) {
  code += "#[repr(" + GenTypeBasic(enum_def.underlying_type) + ")]\n";
  code += "pub enum " + enum_def.name + " {\n";
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end(); ++it) {
    auto &ev = **it;
    code += "  " + ev.name + " = " + NumToString(ev.value) + ",\n";
  }
  if (enum_def.vals.vec.size() == 1) {
    // Needed due to https://github.com/rust-lang/rust/issues/10292
    code += "  Dummy,\n";
  }
  code += "}\n\n";
}

static void GenStruct(StructDef &struct_def, std::string &code) {
  code += "pub struct " + struct_def.name + "<'a>(::flatbuffers::internal::Struct<'a>);\n\n";
  code += "impl<'a> " + struct_def.name + "<'a> {\n";

  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end(); ++it) {
    auto &field = **it;

    code += "  pub fn " + field.name + "(&'a self) -> " +
            GenType(field.value.type) + " {\n";
    code += "    self.0.get(" + NumToString(field.value.offset) + ")\n";
    code += "  }\n\n";
  }
  code += "}\n\n";
}

static void GenTable(StructDef &struct_def, std::string &code) {
  code += "pub struct " + struct_def.name + "<'a>(::flatbuffers::internal::Table<'a>);\n\n";
  code += "impl<'a> " + struct_def.name + "<'a> {\n";

  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end(); ++it) {
    auto &field = **it;
    auto is_scalar = IsScalar(field.value.type.base_type);

    code += "  pub fn " + field.name + "(&'a self) -> " +
            GenType(field.value.type) + " {\n";
    if (is_scalar) {
      code += "    self.0.get_with_default(" + NumToString(field.value.offset) +
              ", " + field.value.constant + ")\n";
    } else {
      code += "    self.0.get(" + NumToString(field.value.offset) + ")\n";
    }
    code += "  }\n\n";
  }
  code += "}\n\n";
}

} // namespace rust

std::string GenerateRust(const Parser &parser) {
  using namespace rust;
  std::string result;

  result += "use flatbuffers::internal::Get;\n\n";

  for (auto it = parser.enums_.vec.begin();
       it != parser.enums_.vec.end(); ++it) {
    GenEnum(**it, result);
  }

  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    if ((**it).fixed) {
      GenStruct(**it, result);
    } else {
      GenTable(**it, result);
    }
  }

  return result;
}

bool GenerateRust(const Parser &parser,
                  const std::string &path,
                  const std::string &file_name) {
  auto code = GenerateRust(parser);

  return !code.length() ||
      SaveFile((path + file_name + ".rs").c_str(), code, false);
}

} // namespace flatbuffers

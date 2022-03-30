// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Path.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

Path::Path(const std::string& fullPath)
{
  mFullPath = fullPath;

  // Separate DO from member path
  const auto separatorPos = mFullPath.find('.');
  if (separatorPos != std::string::npos)
  {
    mDoPath = fullPath.substr(0, separatorPos);
    mMemberPath = fullPath.substr(separatorPos + 1);
  }
  else
  {
    mDoPath = mFullPath;
    mMemberPath = "";
  }
}

const std::string& Path::GetFullDoPath() const { return mDoPath; }

std::string Path::GetSubMemberPath(const std::string& subMember) const
{
  if (mMemberPath.empty())
  {
    return subMember;
  }
  return mMemberPath + "." + subMember;
}

std::string Path::GetFullSubMemberPath(const std::string& subMember) const { return mFullPath + "." + subMember; }

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector

// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Helper class for DO or DO member paths. */
class Path
{
public:
  // Public Methods
  // -------------------

  /*! \brief Instantiates a new DO / DO member path from its full path.
   *  \param fullPath The full DO / DO member path, including its namespaces. */
  Path(const std::string& fullPath);

  /*! \brief Gets the full DO path including the namespace.
   *  \returns the full DO path. */
  const std::string& GetFullDoPath() const;

  /*! \brief Gets the relative sub member path for a DO / DO member.
   *  \param subMember The member for which the path shall be get.
   *  \returns the relative sub member path, e.g. "Member.NestedMember" for the full
   *           path "NS::DO.Member" and the member "NestedMember". */
  std::string GetSubMemberPath(const std::string& subMember) const;

  /*! \brief Gets the full sub member path for a DO / DO member.
   *  \param subMember The member for which the path shall be get.
   *  \returns the full sub member path, e.g. "NS::DO.Member.NestedMember" for the full
   *           path "NS::DO.Member" and the member "NestedMember". */
  std::string GetFullSubMemberPath(const std::string& subMember) const;

private:
  // Private members
  // -------------------

  std::string mFullPath;
  std::string mDoPath;
  std::string mMemberPath;
};

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector

#include<iostream>
#include<string>
#include"list2cpp.hpp"

int main(int argc, char** argv)
{
  if (argc != 2 && argc != 4)
  {
    std::cerr << argv[0] << ": Requires 4 arguments file output varname type (c or s or b)\n Or requires two arguments and reads from stdin and prints to stdout\n$ " << argv[0] << " sourcefile outputfile variablename s\n$ cat sourcefile | " << argv[0] << " varname s\nTo use greater than sorting add a g type variable:\n$ " << argv[0] << " inputfile outputfile varname cg\n" << "For less than add l to type variable like this:\n$ " << argv[0] << " inputfile outputfile varname sl\n";
    exit(EXIT_FAILURE);
  }
  if (argc == 4)
  {
    lst::OutputList str_list;
    std::string arg_chk = argv[2];
    if (str_list.addFile(argv[1]))
    {
      if (str_list.validName(argv[3]) == false)
      {
        std::cerr << "Invalid variable name, must start with a letter and only contain digits and alpha characters.\n";
        exit(EXIT_FAILURE);
      }

      if (str_list.outputToFile(argv[2], argv[3]))
      {
        std::cout << "created " << argv[2] << "\n";
      }
      else
      {
        std::cout << "Encountered an error..\n";
      }
    }
    else
    {
      std::cerr << "Error could not load list from text file: " << argv[1] << "\n";
      exit(EXIT_FAILURE);
    }
  }
  else if (argc == 2)
  {
    lst::OutputList str_list;
    if (str_list.addStream(std::cin))
    {

      if (str_list.validName(argv[1]) == false)
      {
        std::cerr << "Invalid variable name, must start with a letter and only contain digits and alpha characters.\n";
        exit(EXIT_FAILURE);
      }

      if (str_list.outputToFile(std::cout, argv[1]) == false)
      {
        std::cout << "Failed to output stream...\n";
      }
    }
  }
  return 0;
}

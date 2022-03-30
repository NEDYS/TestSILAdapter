using System;
using System.Linq;

namespace FOMCodeGen
{
  class Program
  {
    static void Main(string[] args)
    {
      try
      {
        string filename = "D:/vfs/CANoe/feature/14.0_DistSim_HLA/Projects_Source/CANoe/Source/RTEVENT/DistSim/RTFederate.xml";
        if (args.Count() > 0)
        {
          filename = args[0];
        }
        string enclosingNamespace = "N" + System.IO.Path.GetFileNameWithoutExtension(filename) + "Encoding";
        if (args.Count() > 1)
        {
          enclosingNamespace = args[1];
        }
        bool usePrecompiledHeaders = false;
        if (args.Count() > 2)
        {
          if (args[2] == "-p") usePrecompiledHeaders = true;
        }
        var outputDirectory = System.IO.Path.GetDirectoryName(filename);
        var basename = System.IO.Path.GetFileNameWithoutExtension(filename);
        var encodingsHeaderFilename = System.IO.Path.Combine(outputDirectory, basename + "Encodings.h");
        var encodingsImplFilename = System.IO.Path.Combine(outputDirectory, basename + "Encodings.cpp");
        var dataTypesHeaderFilename = System.IO.Path.Combine(outputDirectory, basename + "DataTypes.h");
        FOMParser fom = new FOMParser(filename, enclosingNamespace);
        fom.UsePrecompiledHeaders = usePrecompiledHeaders;
        FOMDataTypesHeader dataTypesHeader = new FOMDataTypesHeader(fom);
        String dataTypesHeaderHeaderContent = dataTypesHeader.TransformText();
        System.IO.File.WriteAllText(dataTypesHeaderFilename, dataTypesHeaderHeaderContent);

        FOMEncodingHeader encodingsHeader = new FOMEncodingHeader(fom, dataTypesHeaderFilename);
        String encodingsHeaderContent = encodingsHeader.TransformText();
        System.IO.File.WriteAllText(encodingsHeaderFilename, encodingsHeaderContent);

        FOMEncodingImpl encodingsImpl = new FOMEncodingImpl(fom, encodingsHeaderFilename);
        String encodingsImplContent = encodingsImpl.TransformText();
        System.IO.File.WriteAllText(encodingsImplFilename, encodingsImplContent);
      }
      catch (Exception e)
      {
        System.Console.WriteLine("{0}", e.Message);
      }
    }
  }
}

namespace FOMCodeGen
{
  using System.Linq;
  using System.Text;
  using System.Collections.Generic;
  using System;
  public partial class FOMEncodingHeader : FOMEncodingHeaderBase
  {
    FOMParser FOM { get; set; }
    string DataTypesHeaderFilename { get; set; }
    public FOMEncodingHeader(FOMParser fom, string dataTypesHeaderFilename)
    {
      FOM = fom;
      DataTypesHeaderFilename = System.IO.Path.GetFileName(dataTypesHeaderFilename);
    }
  }
  }
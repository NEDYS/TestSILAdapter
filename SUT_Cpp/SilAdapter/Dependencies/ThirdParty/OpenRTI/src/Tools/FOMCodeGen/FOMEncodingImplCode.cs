namespace FOMCodeGen
{
  using System.Linq;
  using System.Text;
  using System.Collections.Generic;
  using System;
  public partial class FOMEncodingImpl : FOMEncodingImplBase
  {
    FOMParser FOM { get; set; }
    string EncodingHeaderFilename { get; set; }
    public FOMEncodingImpl(FOMParser fom, string encodingHeaderFilename)
    {
      FOM = fom;
      EncodingHeaderFilename = System.IO.Path.GetFileName(encodingHeaderFilename);
    }
  }
  }
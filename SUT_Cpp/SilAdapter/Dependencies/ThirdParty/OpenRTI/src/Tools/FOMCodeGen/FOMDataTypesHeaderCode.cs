namespace FOMCodeGen
{
  using System.Linq;
  using System.Text;
  using System.Collections.Generic;
  using System;
  public partial class FOMDataTypesHeader : FOMDataTypesHeaderBase
  {
    FOMParser FOM { get; set; }
    public FOMDataTypesHeader(FOMParser fom)
    {
      FOM = fom;
    }
  }
}
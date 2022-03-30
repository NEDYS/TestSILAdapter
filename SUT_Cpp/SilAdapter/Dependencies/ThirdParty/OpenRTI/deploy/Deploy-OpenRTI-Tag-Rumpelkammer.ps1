$rumpelfiles = "fedtime1516e.dll","OpenRTI.dll","rti1516e.dll","rtinode.exe"

echo "This script downloads the x84/x64 release builds of a (prompted) OpenRTI Tag 
and places the necessary files in the (prompted) Rumpelkammer directories.

The Rumpelkammer svn repos have to be CHECKED OUT BEFOREHAND!

The Rumpelkammer svn repos have to be COMMITTED MANUALLY!

The following files are copied:
${rumpelfiles}
"

$rooturl = "https://vistrpndart1.vi.vector.int:443/artifactory/pnd-rtklinux-generic-dev/OpenRTI/upload/"

$tag = Read-Host -Prompt "Enter OpenRTI Tag (x.x.x.x)"
$shorttag = $tag.Substring(0, $tag.LastIndexOf('.')) 
$tagpath = $shorttag+"/"+$tag+"/"

if ($tag.Split('.').Length -ne 4)
{
	echo "> '${tag}' is not a valid tag, aborting"
	Read-Host -Prompt "Press Enter to exit"
	return
}

$files = "openrti-${tag}-VS-v141-x86-release.zip",
		 "openrti-${tag}-VS-v141-x64-release.zip"
		 

$tmppath = ".\tmp"

$extractpaths =	@{$files[0]="\Exec32";
				  $files[1]="\Exec64"}
				  
$rumpelpath32 = Read-Host -Prompt "Enter path to Rumpelkammer Exec32"
if (!(Test-Path $rumpelpath32 -PathType Container))
{
	echo "> '${rumpelpath32}' does not exist, aborting"
	Read-Host -Prompt "Press Enter to exit"
	return
}
$rumpelpath64 = Read-Host -Prompt "Enter path to Rumpelkammer Exec64"
if (!(Test-Path $rumpelpath64 -PathType Container))
{
	echo "> '${rumpelpath32}' does not exist, aborting"
	Read-Host -Prompt "Press Enter to exit"
	return
}

try {
	$credFile = Get-Content credentials.txt
	$username = $credFile[0]
	$password = $credFile[1]
	echo "> Found credentials for user '${username}'"
}
catch{
	echo "> No credentials.txt found, continue with manual authentication"
	$username = Read-Host -Prompt "Enter Username"
	$password = Read-Host -Prompt "Enter Password" -AsSecureString
}

try {
	$client = new-object system.net.webclient
	$client.credentials = new-object system.net.networkcredential($username, $password)
	foreach($file in $files){
		$url = $rooturl+$tagpath+$file
		$out = $file
		if (Test-Path $out -PathType leaf)
		{
			echo "> File '$out' already exists, skipping download"
		}
		else
		{
			echo "> Downloading '${url}'"
			$client.downloadfile($url,$out)
		}
		
		#New-Item -Path "." -Name $tmppath -ItemType "directory"
		$extractpath = $tmppath+$extractpaths[$file]
		
		echo "> Extracting to '${extractpath}'"
		Expand-Archive -Path $out -DestinationPath $extractpath -Force
		echo "> Removing unneeded files"
		Get-ChildItem -Path $extractpath -Include * -Exclude $rumpelfiles -File -Recurse | foreach { $_.Delete()}
	}
	echo "> Copy files to '${rumpelpath32}'"
	Copy-Item -Path "${tmppath}\Exec32\*" -Destination $rumpelpath32 -Recurse
	echo "> Copy files to '${rumpelpath64}'"
	Copy-Item -Path "${tmppath}\Exec64\*" -Destination $rumpelpath64 -Recurse
	echo "> Cleanup ${tmppath}"
	Remove-Item $tmppath -Recurse
}
catch{
	echo $error[0]
}
finally{
	Read-Host -Prompt "Press Enter to exit"
}
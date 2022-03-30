echo "This script downloads the x84/x64 debug/release builds of a (prompted) OpenRTI Tag. 
Given a destination path, it extracts the files to the corresponding Exec32/Exec32_Debug etc. directories.
The destination path can be the '\bin' folder of a local CANoe branch for local deployment,
or the 'latest' folder for deployment to latest."

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
		 "openrti-${tag}-VS-v141-x64-release.zip",
		 "openrti-${tag}-VS-v141-x86-debug.zip",
		 "openrti-${tag}-VS-v141-x64-debug.zip"

$extractpaths =	@{$files[0]="\Exec32";
				  $files[1]="\Exec64";
				  $files[2]="\Exec32_Debug";
				  $files[3]="\Exec64_Debug"}
				  
$canoepath = Read-Host -Prompt "Enter destination path (e.g. CANoe binaries, latest)"

if (!(Test-Path $canoepath -PathType Container))
{
	echo "> '${canoepath}' does not exist, aborting"
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
		
		$extractpath = $canoepath+$extractpaths[$file]
		
		echo "> Extracting to '${extractpath}'"
		Expand-Archive -Path $out -DestinationPath $extractpath -Force
	}
}
catch{
	echo $error[0]
}
finally{
	Read-Host -Prompt "Press Enter to exit"
}
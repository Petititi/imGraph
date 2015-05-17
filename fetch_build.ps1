[CmdletBinding()]
Param(
	[ValidateScript({Test-Path $_})][String] $CMAKE =				"cmake.exe",
	[ValidateScript({Test-Path $_})][String] $GIT =					"git.exe",
	[ValidateScript({Test-Path $_})][String] $VCVARSALL =			$env:VS120COMNTOOLS+"VsDevCmd.bat",
	[ValidateScript({Test-Path $_})][String] $OPENCV =				"..\opencv",
	[ValidateScript({Test-Path $_})][String] $OPENCV_CONTRIBUTE =	"..\opencv_contrib",
	[ValidateScript({Test-Path $_})][String] $QT =					"..\Qt\5.3\msvc2013_opengl",
	[ValidateScript({Test-Path $_})][String] $INPUT_LOADER =		"..\inputloader",
	[ValidateScript({Test-Path $_})][String] $BOOST =				"..\boost_1_55_0",
	[String] $WINDOWS_SDK = $null,
	[String] $GENERATOR = "Visual Studio 12 2013",
	[Bool] $BUILD_OPENCV = $true,
	[Bool] $BUILD_INPUT_LOADER = $true
)

Begin{
	Invoke-Expression -Command "$CMAKE --version" -ErrorAction Stop | Where { $_ -match "cmake version"}
	Invoke-Expression -Command "$GIT --version" -ErrorAction Stop | Where { $_ -match "git version"}
	
	$OPENCV =            Resolve-Path $OPENCV -ErrorAction Stop
	$OPENCV_CONTRIBUTE = Resolve-Path $OPENCV_CONTRIBUTE -ErrorAction Stop
	$QT =                Resolve-Path $QT -ErrorAction Stop
	$INPUT_LOADER =      Resolve-Path $INPUT_LOADER -ErrorAction Stop
	$BOOST =             Resolve-Path $BOOST -ErrorAction Stop
	
	If (-Not $WINDOWS_SDK) {
		# try to figure out where is windows sdk
		If (Test-Path (Join-Path $env:ProgramFiles "Microsoft SDKs\Windows\v7.0")) {
			$WINDOWS_SDK = Join-Path $env:ProgramFiles "Microsoft SDKs\Windows\v7.0"
		}
		ElseIf (Test-Path (Join-Path $env:ProgramFiles "Microsoft SDKs\Windows\v6.1")) {
			$WINDOWS_SDK = Join-Path $env:ProgramFiles "Microsoft SDKs\Windows\v6.1"
		}
	}
	If (Test-Path $WINDOWS_SDK) {
		Write-Host "WindowsSDK found in $WINDOWS_SDK"
	} Else {
		Write-Host "[Warning] Cannot find WindowsSDK. Please use -WINDOWS_SDK parameter to indicate its location, or install it from http://www.microsoft.com/en-us/download/details.aspx?id=11310. Some components such as VCam won't be installed"
		$WINDOWS_SDK = $null
	}
	
	cmd /c "`"$VCVARSALL`"&set" |
	foreach {
	  if ($_ -match "=") {
		$v = $_.split("=")
		set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
	  }
	}
}

Process{
	$OpencvBin = Join-Path $OPENCV "bin"
	$InputLoaderBin = Join-Path $INPUT_LOADER "bin"
	$ImGraphBin = Join-Path $PSScriptRoot "bin"
	
	$OpencvVsProject = Join-Path $OpencvBin "INSTALL.vcxproj"
	$InputLoaderVsProject = Join-Path $InputLoaderBin "INSTALL.vcxproj"
	$ImGraphVsProject = Join-Path $ImGraphBin "ALL_BUILD.vcxproj"
	
	If ($BUILD_OPENCV) {
		Write-Host "[BUILD OpenCV]" -ForegroundColor Yellow
		Start-Process -FilePath $GIT -ArgumentList "pull" -WorkingDirectory $OPENCV -NoNewWindow -ErrorAction Stop -Wait
		Start-Process -FilePath $GIT -ArgumentList "pull" -WorkingDirectory $OPENCV_CONTRIBUTE -NoNewWindow -ErrorAction Stop -Wait
		$OpencvContributeModules = Join-Path $OPENCV_CONTRIBUTE "modules"
		If (-Not (Test-Path $OpencvBin)) {
			New-Item -ItemType directory -Path $OpencvBin -ErrorAction Stop
		}
		Start-Process -FilePath $CMAKE -ArgumentList "-G `"$GENERATOR`" -DWITH_QT=1 -DBUILD_DOCS=0 -DBUILD_PERF_TESTS=0 -DBUILD_TESTS=0 -DBUILD_opencv_adas=0 -DWITH_CUDA=0 -DBUILD_opencv_cvv=0 -DCMAKE_PREFIX_PATH=`"$QT`" -DOPENCV_EXTRA_MODULES_PATH=`"$OpencvContributeModules`" -DBUILD_opencv_python2=0 -DBUILD_opencv_python3=0 -Wno-dev .." -WorkingDirectory $OpencvBin -NoNewWindow -ErrorAction Stop -Wait
		Start-Process -FilePath "msbuild" -ArgumentList "`"$OpencvVsProject`" /m /nologo /p:Configuration=Debug /p:WarningLevel=0" -NoNewWindow -ErrorAction Stop -Wait
		Start-Process -FilePath "msbuild" -ArgumentList "`"$OpencvVsProject`" /m /nologo /p:Configuration=Release /p:WarningLevel=0" -NoNewWindow -ErrorAction Stop -Wait
	}
	
	If ($BUILD_INPUT_LOADER) {
		Write-Host "[BUILD InputLoader]" -ForegroundColor Yellow
		Start-Process -FilePath $GIT -ArgumentList "pull" -WorkingDirectory $INPUT_LOADER -NoNewWindow -ErrorAction Stop -Wait
		If (-Not (Test-Path $InputLoaderBin)) {
			New-Item -ItemType directory -Path $InputLoaderBin -ErrorAction Stop
		}
		Start-Process -FilePath $CMAKE -ArgumentList "-G `"$GENERATOR`" -DCMAKE_INSTALL_PREFIX=`"$InputLoaderBin`" -DCMAKE_CONFIGURATION_TYPES=Debug;Release -DOpenCV_DIR=`"$OpencvBin`" -DBOOST_ROOT=`"$BOOST`" -DINSTALL_CONFIG_CMAKE_PATH=0 -Wno-dev .." -WorkingDirectory $InputLoaderBin -NoNewWindow -ErrorAction Stop -Wait
		Start-Process -FilePath "msbuild" -ArgumentList "`"$InputLoaderVsProject`" /m /nologo /p:Configuration=Debug" -NoNewWindow -ErrorAction Stop -Wait
		Start-Process -FilePath "msbuild" -ArgumentList "`"$InputLoaderVsProject`" /m /nologo /p:Configuration=Release" -NoNewWindow -ErrorAction Stop -Wait
	}
	
	Write-Host "[BUILD imGraph]" -ForegroundColor Yellow
	Start-Process -FilePath $GIT -ArgumentList "pull" -WorkingDirectory $PSScriptRoot -NoNewWindow -ErrorAction Stop -Wait
	If (-Not (Test-Path $ImGraphBin)) {
		New-Item -ItemType directory -Path $ImGraphBin -ErrorAction Stop
	}
	$ImGraphCmakeParams = "-DCMAKE_INSTALL_PREFIX=`"$ImGraphBin`" -DCMAKE_CONFIGURATION_TYPES=Debug;Release -DOpenCV_DIR=`"$OpencvBin`" -DCMAKE_PREFIX_PATH=`"$QT;$InputLoaderBin`" -DBOOST_ROOT=`"$BOOST`" -Wno-dev"
	If ($WINDOWS_SDK) {
		#build directshow samples if not exist
		If (-Not (Test-Path (Join-Path $WINDOWS_SDK "Samples\Multimedia\DirectShow\BaseClasses\Debug\strmbasd.lib"))) {
			Start-Process -FilePath "msbuild" -ArgumentList ("`"{0}`" /m /nologo /p:Configuration=Debug" -f (Join-Path $WINDOWS_SDK "Samples\Multimedia\DirectShow\BaseClasses\baseclasses.sln")) -Verb runAs -ErrorAction Stop -Wait
		}
		If (-Not (Test-Path (Join-Path $WINDOWS_SDK "Samples\Multimedia\DirectShow\BaseClasses\Release\strmbase.lib")))	{
			Start-Process -FilePath "msbuild" -ArgumentList ("`"{0}`" /m /nologo /p:Configuration=Release" -f (Join-Path $WINDOWS_SDK "Samples\Multimedia\DirectShow\BaseClasses\baseclasses.sln")) -Verb runAs -ErrorAction Stop -Wait
		}
		$ImGraphCmakeParams += " -DWINDOWS_SDK_DIR=`"$WINDOWS_SDK`""
	}
	Start-Process -FilePath $CMAKE -ArgumentList "-G `"$GENERATOR`" $ImGraphCmakeParams .." -WorkingDirectory $ImGraphBin -NoNewWindow -ErrorAction Stop -Wait
	#Start-Process -FilePath "msbuild" -ArgumentList "`"$ImGraphVsProject`" /m /nologo /p:Configuration=Debug" -NoNewWindow -ErrorAction Stop -Wait
	#Start-Process -FilePath "msbuild" -ArgumentList "`"$ImGraphVsProject`" /m /nologo /p:Configuration=Release" -NoNewWindow -ErrorAction Stop -Wait
}
End{}
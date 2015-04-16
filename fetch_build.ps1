[CmdletBinding()]
Param(
	[ValidateScript({Test-Path $_})][String] $CMAKE =				"cmake.exe",
	[ValidateScript({Test-Path $_})][String] $GIT =					"git.exe",
	[ValidateScript({Test-Path $_})][String] $VCVARSALL =			$env:VS120COMNTOOLS+"VsDevCmd.bat",
	[ValidateScript({Test-Path $_})][String] $OPENCV =				"..\opencv",
	[ValidateScript({Test-Path $_})][String] $OPENCV_CONTRIBUTE =	"..\opencv_contrib",
	[ValidateScript({Test-Path $_})][String] $QT =					"..\Qt\Qt5.3.2\5.3\msvc2013_opengl",
	[ValidateScript({Test-Path $_})][String] $INPUT_LOADER =		"..\inputloader",
	[ValidateScript({Test-Path $_})][String] $BOOST =				"..\boost_1_55_0",
	[String] $GENERATOR = "Visual Studio 12 2013"
	
)

Begin{
	Invoke-Expression -Command "$CMAKE --version" -ErrorAction Stop | Where { $_ -match "cmake version"}
	Invoke-Expression -Command "$GIT --version" -ErrorAction Stop | Where { $_ -match "git version"}
	
	$OPENCV =            Resolve-Path $OPENCV
	$OPENCV_CONTRIBUTE = Resolve-Path $OPENCV_CONTRIBUTE
	$QT =                Resolve-Path $QT
	$INPUT_LOADER =      Resolve-Path $INPUT_LOADER
	$BOOST =             Resolve-Path $BOOST
	
	cmd /c "`"$VCVARSALL`"&set" |
	foreach {
	  if ($_ -match "=") {
		$v = $_.split("=")
		set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
	  }
	}
}

Process{
	Write-Host "[BUILD OpenCV]" -ForegroundColor Yellow
	Start-Process -FilePath $GIT -ArgumentList "pull" -WorkingDirectory $OPENCV -NoNewWindow -ErrorAction Stop -Wait
	Start-Process -FilePath $GIT -ArgumentList "pull" -WorkingDirectory $OPENCV_CONTRIBUTE -NoNewWindow -ErrorAction Stop -Wait
	$OpencvContributeModules = Join-Path $OPENCV_CONTRIBUTE "modules"
	$OpencvBin = Join-Path $OPENCV "bin"
	If (-Not (Test-Path $OpencvBin)) {
		New-Item -ItemType directory -Path $OpencvBin -ErrorAction Stop
	}
	Start-Process -FilePath $CMAKE -ArgumentList "-G `"$GENERATOR`" -DWITH_QT=1 -DBUILD_DOCS=0 -DBUILD_PERF_TESTS=0 -DBUILD_TESTS=0 -DBUILD_opencv_adas=0 -DWITH_CUDA=0 -DBUILD_opencv_cvv=0 -DCMAKE_PREFIX_PATH=`"$QT`" -DOPENCV_EXTRA_MODULES_PATH=`"$OpencvContributeModules`" -DBUILD_opencv_python2=0 -DBUILD_opencv_python3=0 -Wno-dev .." -WorkingDirectory $OpencvBin -NoNewWindow -ErrorAction Stop -Wait
	$OpencvVsProject = Join-Path $OpencvBin "OpenCV.sln"
	Start-Process -FilePath "msbuild" -ArgumentList "`"$OpencvVsProject`" /m /nologo /p:Configuration=Debug /p:WarningLevel=0" -NoNewWindow -ErrorAction Stop -Wait
	Start-Process -FilePath "msbuild" -ArgumentList "`"$OpencvVsProject`" /m /nologo /p:Configuration=Release /p:WarningLevel=0" -NoNewWindow -ErrorAction Stop -Wait
	
	Write-Host "[BUILD InputLoader]" -ForegroundColor Yellow
	Start-Process -FilePath $GIT -ArgumentList "pull" -WorkingDirectory $INPUT_LOADER -NoNewWindow -ErrorAction Stop -Wait
	$InputLoaderBin = Join-Path $INPUT_LOADER "bin"
	If (-Not (Test-Path $InputLoaderBin)) {
		New-Item -ItemType directory -Path $InputLoaderBin -ErrorAction Stop
	}
	Start-Process -FilePath $CMAKE -ArgumentList "-G `"$GENERATOR`" -DCMAKE_INSTALL_PREFIX=`"$InputLoaderBin`" -DCMAKE_CONFIGURATION_TYPES=Debug;Release -DOpenCV_DIR=`"$OPENCV`" -Wno-dev .." -WorkingDirectory $InputLoaderBin -NoNewWindow -ErrorAction Stop -Wait
	$InputLoaderVsProject = Join-Path $InputLoaderBin "InputLoader.sln"
	Start-Process -FilePath "msbuild" -ArgumentList "`"$InputLoaderVsProject`" /m /nologo /p:Configuration=Debug" -NoNewWindow -ErrorAction Stop -Wait
	Start-Process -FilePath "msbuild" -ArgumentList "`"$InputLoaderVsProject`" /m /nologo /p:Configuration=Release" -NoNewWindow -ErrorAction Stop -Wait
	
	Write-Host "[BUILD imGraph]" -ForegroundColor Yellow
	Start-Process -FilePath $GIT -ArgumentList "pull" -WorkingDirectory $PSScriptRoot -NoNewWindow -ErrorAction Stop -Wait
	$ImGraphBin = Join-Path $PSScriptRoot "bin"
	If (-Not (Test-Path $ImGraphBin)) {
		New-Item -ItemType directory -Path $ImGraphBin -ErrorAction Stop
	}
	Start-Process -FilePath $CMAKE -ArgumentList "-G `"$GENERATOR`" -DCMAKE_INSTALL_PREFIX=`"$ImGraphBin`" -DCMAKE_CONFIGURATION_TYPES=Debug;Release -DOpenCV_DIR=`"$OPENCV`" -DCMAKE_PREFIX_PATH=`"$QT;$INPUT_LOADER`" -DBOOST_ROOT=`"$BOOST`" -Wno-dev .." -WorkingDirectory $ImGraphBin -NoNewWindow -ErrorAction Stop -Wait
	$ImGraphVsProject = Join-Path $ImGraphBin "imGraph.sln"
	Start-Process -FilePath "msbuild" -ArgumentList "`"$ImGraphVsProject`" /m /nologo /p:Configuration=Debug" -NoNewWindow -ErrorAction Stop -Wait
	Start-Process -FilePath "msbuild" -ArgumentList "`"$ImGraphVsProject`" /m /nologo /p:Configuration=Release" -NoNewWindow -ErrorAction Stop -Wait
}
End{}
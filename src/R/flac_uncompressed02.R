library(sys)

codearguments <- commandArgs(trailingOnly = TRUE)

rootdir <- print(codearguments[1], quote = TRUE)
basedir <- paste(rootdir, codearguments[2], sep = "", collapse = NULL)
targetdir <- paste(rootdir, codearguments[3], sep = "", collapse = NULL)

basefileindex <- as.numeric(codearguments[4])
spectrogramoption <- as.numeric(codearguments[5])

logfn <- function(nflag,datavector,target){
        if (nflag == 1) {
            write(datavector,target,ncolumns = 1,append = FALSE, sep = "\n")
        } else {
            write(datavector,target,ncolumns = 1,append = TRUE, sep = "\n")
        }
}

startts <- Sys.time()

basefiles_flac <- list.files(
                path = basedir,
                pattern = "\\.flac$",
                all.files = TRUE,
                full.names = TRUE,
                recursive = TRUE,
                ignore.case = FALSE,
                include.dirs = FALSE,
                no.. = FALSE
        )
basefiles_wav <- list.files(
                path = basedir,
                pattern = "\\.wav$",
                all.files = TRUE,
                full.names = TRUE,
                recursive = TRUE,
                ignore.case = FALSE,
                include.dirs = FALSE,
                no.. = FALSE
        )
basefiles_aif <- list.files(
                path = basedir,
                pattern = "\\.aif$",
                all.files = TRUE,
                full.names = TRUE,
                recursive = TRUE,
                ignore.case = FALSE,
                include.dirs = FALSE,
                no.. = FALSE
        )
basefiles_aiff <- list.files(
                path = basedir,
                pattern = "\\.aiff$",
                all.files = TRUE,
                full.names = TRUE,
                recursive = TRUE,
                ignore.case = FALSE,
                include.dirs = FALSE,
                no.. = FALSE
        )

basefiles01 <- c(basefiles_flac,
		basefiles_wav,
		basefiles_aif,
		basefiles_aiff
	)

basefile02 <- basefiles01[basefileindex]

rm(
basefiles_flac,
basefiles_wav,
basefiles_aif,
basefiles_aiff,
basefiles01,
indexfile,
basefileindex
)

targetfile <- gsub(basedir, targetdir, basefile02, ignore.case = FALSE,
		perl = FALSE, fixed = FALSE, useBytes = FALSE
	)

targetfiledir <- sub("/[^/]*$", "",targetfile)
targetfilelogdir <- paste(targetfiledir, "/logsflac", sep = "", collapse = NULL)
targetfilespectrogramsdir <- paste(targetfiledir, "/spectrograms", sep = "", collapse = NULL)
targetfilename <- sub(".*\\/", "", targetfile)

formatfn <- function(x01){
    filename <- substr(x01, 1, nchar(x01)-4)
    format <- substr(x01, nchar(x01)-4+1, nchar(x01))
    filename <- if (format == ".wav") {
                substr(x01, 1, nchar(x01)-3)
                } else if (format == ".aif") {
                substr(x01, 1, nchar(x01)-3)
                } else {
                substr(x01, 1, nchar(x01)-4)
                }
    format <- if (format == ".wav") {
                gsub("\\.","",format)
                } else if (format == ".aif") {
                gsub("\\.","",format)
                } else {
                substr(x01, nchar(x01)-4+1, nchar(x01))
                }
    c(filename,format)
}

filemd <- paste(targetfilelogdir,"/",formatfn(targetfilename)[1], "md", sep = "", collapse = NULL)
filetsmd <- paste(targetfilelogdir,"/",formatfn(targetfilename)[1],"ts", "md", sep = "", collapse = NULL)
filepng <- paste(targetfilespectrogramsdir,"/",formatfn(targetfilename)[1], "png", sep = "", collapse = NULL)
fileflac <- paste(targetfiledir,"/",formatfn(targetfilename)[1], "mp3", sep = "", collapse = NULL)
fileformat <- formatfn(targetfilename)[2]

ffmpegfn <- function(x,n){
    argsffmpeg <- c(
                    "-nostdin",
                    "-i",
                    x,
                    "-c:a libmp3lame",
                    "-b:a 320k",
                    n
                    )
    exec_wait("/usr/bin/ffmpeg",
        args = argsffmpeg,
        std_out = FALSE,
        std_err = FALSE,
        std_in = NULL,
        timeout = 0
    )
}

ffmpegfn(basefile02,fileflac)

spectrogramfn <- function(x,n){
        argsffmpeg <- c(
                        "-nostdin",
                        "-i",
                        x,
                        "-lavfi",
                        "showspectrumpic",
                        n
                      )
        exec_wait("/usr/bin/ffmpeg",
            args = argsffmpeg,
            std_out = FALSE,
            std_err = FALSE,
            std_in = NULL,
            timeout = 0
        )
}

if (spectrogramoption == 1) {
spectrogramfn(fileflac,filepng)
}

exiftoolfn <- function(e,k){
        argsexiftool <- c(
                        "-a",
                        "-G1",
                        "-s",
                        e
                        )
        exec_wait(
                "/usr/bin/exiftool",
                args = argsexiftool,
                std_out = k,
                std_err = FALSE,
                std_in = NULL,
                timeout = 0
        )
}

exiftoolfn(fileflac,filemd)
finishts <- Sys.time()
timestamps <- c(startts,finishts)

logfn(1,timestamps,filetsmd)
q(save = "no") 

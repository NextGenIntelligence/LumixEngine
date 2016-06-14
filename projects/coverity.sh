PLATFORM=`uname`
TOOL_ARCHIVE=/tmp/cov-analysis-${PLATFORM}.tgz
TOOL_URL=https://scan.coverity.com/download/${PLATFORM}
TOOL_BASE=/tmp/coverity-scan-analysis

echo ${PLATFORM}

if [ ! -d $TOOL_BASE ]; then
  # Download Coverity Scan Analysis Tool
  if [ ! -e $TOOL_ARCHIVE ]; then
    curl -o $TOOL_ARCHIVE $TOOL_URL --data "project=$COVERITY_SCAN_PROJECT_NAME&token=$COVERITY_SCAN_TOKEN"
    echo -e "\033[33;1mDownloading Coverity Scan Analysis Tool...\033[0m"
  fi
fi
#!/bin/bash
SEMANTIC=`cat symantic.version`

function trim() { echo $1; }

if [ -z $GIT_HOME ]; then
  if [ -f "/usr/local/bin/git" ]; then
    export GIT_HOME=/usr/local/bin/git
  elif [ -f "/usr/bin/git" ]; then
    export GIT_HOME=/usr/bin/git
  elif [ -f "/bin/git" ]; then
    export GIT_HOME=/bin/git
  else
    echo "Could not find git."
    exit 1
  fi
fi

export GIT_REV_COUNT_RAW=`$GIT_HOME log --pretty=oneline | wc -l`
export GIT_REV_COUNT=`trim $GIT_REV_COUNT_RAW`
export GIT_REV=`$GIT_HOME rev-parse HEAD`

VERSION=$SEMANTIC.$TAG$GIT_REV_COUNT

echo "$VERSION"
#!/bin/bash

PROJECT="SkyrimNXModManager"
YEAR="2019"
AUTHOR="Max Roncace"
EMAIL="mproncace@gmail.com"

# read the header template
header=$(cat ./res/HEADER.txt)

# apply replacements
header=$(sed -r "s/\\$\{project\}/$PROJECT/" <<< "$header")
header=$(sed -r "s/\\$\{year\}/$YEAR/" <<< "$header")
header=$(sed -r "s/\\$\{author\}/$AUTHOR/" <<< "$header")
header=$(sed -r "s/\\$\{email\}/$EMAIL/" <<< "$header")

# add an asterisk to the start of each line
header=$(sed -r "s/^(.)/ * \1/" <<< "$header")
# separate command for empty lines to avoid trailing spaces
header=$(sed -r "s/^$/ */" <<< "$header")

# add /* */ to the start and end
header=$(printf "/*"'\n'"$header"'\n'" */")

# variable to track how many files are updated
count=0

# discover all source/header files
find "./include" "./src" -type f \( -iname "*.c" -or -iname "*.h" -or -iname "*.cpp" -or -iname "*.hpp" \) -print0 | \
        while IFS= read -r -d $'\0' file; do
    #escape asterisks in the header
    header_esc=$(sed 's/[\*]/\\&/g' <<< "$header")

    # read in the file and check if it starts with the header
    if [[ $(cat "$file") == $header_esc* ]]; then
        continue
    fi

    count=$((count+1))

    if grep -Pzoq "(?s)\/\*.*Copyright \(c\).*?\*\/" "$file"; then
        echo "Updating header for file $file"

        # adapted from https://stackoverflow.com/a/21702566
        # this basically replaces the license pattern with our current formatted license,
        # with a neat trick to deal with a multi-line context
        # we also append a dot to preserve trailing whitespace during substitution
        output=$(gawk -v RS='^$' -v hdr="$header\n\n" '{sub(/\/\*.*?Copyright \(c\).*?\*\/\n*/,hdr)}1 {print $0}' $file && echo .)
    else
        echo "Generating new header for file $file"
        
        # save output to intermediate variable with trailing character
        # this prevents bash from stripping trailing newlines
        output="$header"$'\n'$'\n'"$(cat $file && echo .)"
    fi

    # escape any formatting sequences present in the code since printf likes to convert them
    output="${output//%/%%}"
    # escape previously-escaped newlines since printf likes to convert them too
    output="${output//\\n/\\\\n}"
    # remove the trailing dot we added
    output="${output:0:-1}"
    # finally, write it to disk!
    printf "$output" > $file
done

# if we didn't update anything, say so (to avoid zero-output)
if [ $count -eq 0 ]; then
    echo "All file headers up-to-date"
fi

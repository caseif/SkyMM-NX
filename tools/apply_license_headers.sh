#!/bin/bash

PROJECT="SkyMM-NX"
YEAR="2019"
AUTHOR="Max Roncace"
EMAIL="mproncace@gmail.com"

hdr_file="./res/HEADER.txt"

# read the header template
header=$(cat $hdr_file)

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

# add two trailing newlines for one blank line of padding after the header
header=$header$'\n'$'\n'

# escape the @ symbol in the header (from the email address) since Perl doesn't like it
# the "pf" stands for Perl-friendly
header_pf=${header//\@/\\@}

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

        # we use a relatively simple Perl script to replace the existing copyright notice
        # we append a newline to the end since Perl strips it
        # we also append a dot to preserve the newline during substitution
        output=$(perl -p0e "sprintf s/\/\*.*?Copyright \(c\).*?\*\/\n*/${header_pf//\//\\/}/s" $file)$'\n'"."
    else
        echo "Generating new header for file $file"
        
        # save output to intermediate variable with trailing character
        # this prevents bash from stripping trailing newlines
        output="$header""$(cat $file && echo .)"
    fi

    # escape any formatting sequences present in the code since printf likes to convert them
    output="${output//%/%%}"
    # escape previously-escaped newlines since printf likes to convert them too
    output="${output//\\/\\\\}"
    # remove the trailing dot we added (and the trailing newline if required)
    output="${output:0:-1}"
    # finally, write it to disk!
    printf "$output" > $file
done

# if we didn't update anything, say so (to avoid zero-output)
if [ $count -eq 0 ]; then
    echo "All file headers up-to-date"
fi

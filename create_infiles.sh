#!/bin/bash

echo Number of input parameters = $# 

dir_name=$1
num_of_files=$2
num_of_dirs=$3
levels=$4

#echo $num_of_files
number='^[0-9]+$'

## params

if [ "$#" -ne 4 ]; then
	echo "Arguments must be 4 , Please try again ..." 
	exit 1
fi

if ! [[ $num_of_files =~ $number ]]; then
	echo "Error , num_of_files must be a number , Please try again"
	exit 1
fi

if ! [[ $num_of_dirs =~ $number ]]; then
	echo "Error , num_of_dirs must be a number , Please try again"
	exit 1	
fi

if ! [[ $levels =~ $number ]]; then
	echo "Error , levels must be a number , Please try again"
	exit 1
fi


##############################################################################
                    ##Create the main directory

if [ ! -d "$dir_name" ];then 
	echo "Directory with name $dir_name doesn't exist ."
	mkdir $dir_name
	echo "We just created the root ${dir_name}"
else
	echo "Directory allready exists"
fi

################################################################################
					##Create Subdirectories
echo "                                               "

declare -a directories
declare -a paths
paths[0]=${dir_name}
flag=$levels

for ((i=1; i<=$num_of_dirs; i++ ))
	do
		directories[i]="$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c 8 ; echo '')"

		if (($flag== $levels)); then
			mkdir "${dir_name}/${directories[i]}"
			paths[i]="${dir_name}/${directories[i]}"
			echo "We just created this ${directories[i]} directory"
			flag=1
		else
			for (( j=0; j<=flag; j++ ))
				do
					path+="${directories[i-$flag+j]}/"   ## back to the right path
				done
			paths[i]="${dir_name}/$path"	
			mkdir "${dir_name}/$path"
			echo "We just created this ${directories[i]} directory"
			path=""
			flag=$((flag+1))
		fi
	done


#################################################################################
							##Create-fill-deliver file names

declare -a files
for (( i=0; i<=$num_of_files-1; i++ ))
	do
		files[i]="$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c 8 ; echo '')"
	done

flag2=0
for (( i=0; i<=$num_of_files-1; i++))

	do
		if (($flag2==$num_of_dirs)); then
			echo >${paths[flag2]}/${files[i]}
			f=${paths[flag2]}/${files[i]}
			echo "$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c 4 ; echo '')" > $f
			flag2=0
		else
			echo >${paths[flag2]}/${files[i]}
			f=${paths[flag2]}/${files[i]}
			echo "$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c 4 ; echo '')" > $f

		fi

		flag2=$((flag2+1))
	done

################################################################################



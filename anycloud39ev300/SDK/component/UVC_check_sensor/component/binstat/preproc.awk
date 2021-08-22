BEGIN{
}

function get_name(s,   subs)
{
	n = split(s,array,"/")
	subs = array[n]
	sub(/\(.*\)/, "", subs)
	return subs
}

{
if(NF == 1){
	if(match($1, /^\./)){
		section=$1;
		getline;
		if(NF == 3){
			fname=get_name($3);
			print section "\t\t" $2 "\t\t" fname
		}
	}
}
else if(NF == 4) {
	if(match($1, /^\./) || match($1, /COMMON/)){
		fname=get_name($4);
		print $1 "\t\t" $3 "\t\t" fname
	}
}
else if(NF == 3) {
	if(match($1, /fill/)){
		print $1 "\t\t" $3 "\t\tfill"
	}
}

}

END{
}

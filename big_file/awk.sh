awk '

BEGIN {
	count=0;
	for (i = 0; i < 8; ++i)
		gc_last[i] = 0;
	for (i = 0; i < 7; ++i)
		cp_last[i] = 0;
}
{
	if (($1 == "write:" || $1 == "read:" ) && match($3, "BW")) {
		gsub("BW=", "", $3);
		split($3,a,"[A-Z]");
		gsub("[0-9]","",$3);
		gsub(",","",$3);
		gsub("\\.","",$3);
		
		if ($3 == "M")
			lat[0, count] = a[1] * 1024;
		else
			lat[0, count] = a[1];
	}

	if($1 == "lat" && match($3, "min")) {
		gsub(/min=/,"", $3);
		gsub(/max=/,"", $4);
		gsub(/avg=/,"", $5);
		
		lat[1, count]=strtonum($3);
		lat[2, count]=strtonum($4);
		if (match($4, "K"))
			lat[2, count] *= 1000;
		lat[3, count]=strtonum($5);
		count++;
	}
}

END {
	for(i = 0; i < count; ++i) {
		for (j = 0; j < 4; ++j) {
			if (j == 0)
				printf("%f ", lat[j, i]);
			else
				printf("%d ", lat[j, i]);
		}
		printf("\n");
	}
}
' $1


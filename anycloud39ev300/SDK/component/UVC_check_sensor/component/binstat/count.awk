BEGIN{
	count=0
	text_size = 0
	rodata_size = 0
	data_size = 0
}

{
	for(x=1; x<=count; x++)
       	{
		if(array_section[x] == $1 && array_file[x] == $3)
		{
			array_size[x] = array_size[x] + $2;
			break;
		}
	}
	if(x > count)
	{
		++count;
		array_section[count] = $1;
		array_file[count] = $3;
		array_size[count] = $2;
	}
}

END{
	for(x=1; x<=count; x++)
	{
		if(match(array_section[x], /\.text/)){
			text_size = text_size + array_size[x];
		}
		else if(match(array_section[x], /\.rodata/)){
		    rodata_size = rodata_size + array_size[x];
		}
		else if(match(array_section[x], /\.data/)){
		    data_size = data_size + array_size[x];
		}
	}
	
	printf("text: 0x%x\n", text_size);
	printf("rodata: 0x%x\n", rodata_size);
	printf("data: 0x%x\n", data_size);

    printf("\n");
    
	for(x=1; x<=count; x++)
	{
		printf("%-20s\t", array_section[x]);
		printf("0x%-8x\t", array_size[x]);
		printf("%s\n", array_file[x]);
	}
}

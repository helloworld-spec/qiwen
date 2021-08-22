BEGIN{
	count=0
}

{
    if(match($1, /text|data/))
    {
    	for(x=1; x<=count; x++)
        {
    		if(array_file[x] == $3)
    		{
    		    if(match($1, /text/))
    		    {
    			    array_size_text[x] = array_size_text[x] + strtonum($2);
   		        }
                else if(match($1, /rodata/))
                {
    			    array_size_rodata[x] = array_size_rodata[x] + strtonum($2);
                }
                else if(match($1, /\.data/))
                {
    			    array_size_data[x] = array_size_data[x] + strtonum($2);
                }
        		break;
    		}
    	}
    	
    	if(x > count)
    	{
    		++count;
    		array_file[count] = $3;
    		array_size_text[count] = 0;
    		array_size_rodata[count] = 0;
    		array_size_rodata[count] = 0;

		    if(match($1, /text/))
		    {
			    array_size_text[x] = strtonum($2);
		    }
            else if(match($1, /rodata/))
            {
			    array_size_rodata[x] = strtonum($2);
            }
            else if(match($1, /\.data/))
            {
			    array_size_data[x] =  strtonum($2);
            }
    	}
    }
}

END{

	for(x=1; x<=count; x++)
	{
		printf("%-40s\t", array_file[x]);
		printf("%-8d\t", array_size_text[x]);
		printf("%-8d\t", array_size_rodata[x]);
		printf("%-8d\t", array_size_data[x]);

        all = array_size_text[x] + array_size_rodata[x] + array_size_data[x];
		printf("%-8d\n", all);
	}
}

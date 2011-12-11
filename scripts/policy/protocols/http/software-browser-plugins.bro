##! This script take advantage of a few ways that installed plugin information
##! leaks from web browsers.

@load base/protocols/http
@load base/frameworks/software

module HTTP;

export {
	redef record Info += {
		## Indicates if the server is an omniture advertising server.
		omniture: bool &default=F;
	};
	
	redef enum Software::Type += {
		BROWSER_PLUGIN
	};
}

event http_header(c: connection, is_orig: bool, name: string, value: string) &priority=3
	{
	if ( is_orig )
		{
		if ( name == "X-FLASH-VERSION" )
			{
			# Flash doesn't include it's name so we'll add it here since it 
			# simplifies the version parsing.
			value = cat("Flash/", value);
			local flash_version = Software::parse(value, c$id$orig_h, BROWSER_PLUGIN);
			Software::found(c$id, flash_version);
			}
		}
	else
		{
		# Find if the server is Omniture
		if ( name == "SERVER" && /^Omniture/ in value )
			c$http$omniture = T;
		}
	}

event log_http(rec: Info)
	{
	# We only want to inspect requests that were sent to omniture advertising 
	# servers.
	if ( rec$omniture && rec?$uri )
		{
		# We do {5,} because sometimes we see p=6 in the urls.
		local parts = split_n(rec$uri, /&p=([^&]{5,});&/, T, 1);
		if ( 2 in parts )
			{
			# We do sub_bytes here just to remove the extra extracted 
			# characters from the regex split above.
			local sw = sub_bytes(parts[2], 4, |parts[2]|-5);
			local plugins = split(sw, /[[:blank:]]*;[[:blank:]]*/);
			
			for ( i in plugins )
				Software::found(rec$id, Software::parse(plugins[i], rec$id$orig_h, BROWSER_PLUGIN));
			}
		}
	}


unsigned char nmea_checksum(unsigned char *s) {

	unsigned char checksum;
	
	if (*s == '$') {
		s++;
	}
	
	while (*s && *s != '*') {
		checksum ^= *s;
		s++;
	}				
	
	return checksum;
}
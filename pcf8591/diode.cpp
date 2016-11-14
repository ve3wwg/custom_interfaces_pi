//////////////////////////////////////////////////////////////////////
// diode.cpp
// Date: Tue Aug 23 19:47:58 2016  (C) Warren W. Gay VE3WWG 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

static const char *i2c_device = "/dev/i2c-1";
static int i2c_fd = -1;
static int i2c_addr = 0x48;
static const int ain1 = 1;	// Measure AOUT
static const int ain0 = 0;	// Measure Diode

int
i2c_write(int addr,uint8_t byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[1];
	uint8_t txbuf[2];
	int rc;

	txbuf[0] = 0b01000000;		// Enable DAC
	txbuf[1] = byte;		// DAC value to write

	iomsgs[0].addr = unsigned(addr);
	iomsgs[0].flags = 0;		// Write
	iomsgs[0].buf = txbuf;
	iomsgs[0].len = 2;

	msgset.msgs = iomsgs;
	msgset.nmsgs = 1;

	rc = ioctl(i2c_fd,I2C_RDWR,&msgset);
	return rc < 0 ? -1 : 0;
}

int
i2c_read(int addr,uint8_t control,uint8_t& read_byte) {
	struct i2c_rdwr_ioctl_data msgset;
	struct i2c_msg iomsgs[2];
	uint8_t rxbuf[3];
	int rc;

	control |= 0b01000000;		// Keep DAC enabled

	iomsgs[0].addr = iomsgs[1].addr = unsigned(addr);
	iomsgs[0].flags = 0;		// Write
	iomsgs[0].buf = &control;
	iomsgs[0].len = 1;

	iomsgs[1].flags = I2C_M_RD;	// Read
	iomsgs[1].buf = rxbuf;
	iomsgs[1].len = 3;

	msgset.msgs = iomsgs;
	msgset.nmsgs = 2;

	rc = ioctl(i2c_fd,I2C_RDWR,&msgset);
	read_byte = rxbuf[2];
	return rc < 0 ? -1 : 0;
}

int
main(int argc,char **argv) {
	int rc = 0, optch;
	uint8_t read0, read1;
	double VD1, VR, I;

	while ( (optch = getopt(argc,argv,"a:h")) != -1) {
		switch ( optch ) {
		case 'h' :
			printf(
				"%s [-a address] [-h]\n"
				"where:\n"
				"\t-a address\tSpecify I2C address\n"
				"\t-h\t\tHelp\n",
				argv[0]);
			exit(0);
		case 'a' :
			i2c_addr = strtol(optarg,0,0);
			break;
		default :
			fprintf(stderr,"Unsupported option: -%c\n",optch);
			rc = 1;
		}
	}
	
	if ( rc )
		exit(1);

	{
		FILE *gnuplot = fopen("gnuplot.cmd","w");

		fputs(	"set title \"Diode 1N914 Plot\"\n"
			"set xlabel \"Voltage across D1\"\n"
			"set ylabel \"Current through D1\"\n"
			"set autoscale\n"
			"set yrange [0:0.0008]\n"
			"set xrange [0:0.7]\n"
			"plot \"diode.dat\" using 1:2 with lines\n",
			gnuplot);
		fclose(gnuplot);
	}

	FILE *plot = fopen("diode.dat","w");

	i2c_fd = open(i2c_device,O_RDWR);
	if ( i2c_fd == -1 ) {
		fprintf(stderr,"%s: opening %s\n",
			strerror(errno),
			i2c_device);
		exit(1);
	}

	for ( unsigned dac=0; dac <= 0xFF; ++dac ) {
		rc = i2c_write(i2c_addr,dac);	// Write DAC value
		assert(!rc);

		usleep(10000);

		rc = i2c_read(i2c_addr,ain0,read0);
		assert(!rc);

		rc = i2c_read(i2c_addr,ain1,read1);
		assert(!rc);

		VD1 = read0 * 3.3 / 256.0;	// Voltage across D1
		if ( (VR = read1 - read0) < 0 )
			VR = 0;			// Don't allow V(R) go negative
		VR = VR * 3.3 / 256.0;		// In volts
		I = VR / 3300.0;		// Currnet in Amperes
		fprintf(plot,"%lf %lf\n",VD1,I);

		printf("DAC %u, AIN0 %u, AIN1 %u, VD1=%.1lf, VR=%.1lf, I=%.4lf\n",dac,read0,read1,VD1,VR,I);
	}

	fclose(plot);
	close(i2c_fd);
	return 0;
}

// End diode.cpp

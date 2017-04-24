           Linux VoyagerGX Serial driver information
           =========================================


Introduction
------------
Silicon Motion, Inc. VoyagerGX serial driver supports 16750 model UART device,
and it utilizes two 64-byte FIFOs, one for reception and one for transmission.
The driver supports the following baud rates.

   - 50 bps	- 1200 bps	- 9600 bps	- 57600 bps
   - 75 bps	- 1800 bps	- 12800 bps	- 115200 bps
   - 110 bps	- 2000 bps	- 14400 bps	- 128000 bps
   - 135 bps	- 2400 bps	- 19200 bps	- 256000 bps
   - 150 bps	- 3600 bps	- 23040 bps
   - 300 bps	- 4800 bps	- 28800 bps
   - 600 bps	- 7200 bps	- 38400 bps


Supported Kernel Version
------------------------
The driver has been developed and tested under Linux 2.4.20.


Setup
-----
VoyagerGX serial driver initial setup information for Linux:

* Copy the following device driver files to <linux source root>/drivers/char/

    Config.in
    Makefile
    smivgx.h
    smivgx_duart.c
    tty_io.c

* Use your favorite config program (menuconfig or xconfig) to configure the
  driver. Go under "Characher devices", where you should find the following:

    Silicon Motion VoyagerGX (SM501) serial port
      SMI VoyagerGX console support
      Output buffer size (in bytes)

  The first line enables the driver, this one is a must. The second line
  enables the console support for the driver. And finally you can change
  the default size of the internal transmit buffer. The default size is
  1024 bytes, which in most cases should be good eniugh.
  After you're done with the configuration steps, save the changes abd exit
  from the config program.

  When the configuration is complete, run consequently these commands:

    make dep
    make clean
    make bzImage

  If you haven't got any error messages, at this point you shoud have
  a compressed kernel image file ready.


Errata
------

* The current Voyager GX Serial device does not work properly with the
  standard serial device due to discrepancy in baud rate generator clock
  where SMI clock can not get the actual baud rate.

* Voyager Gx can only supports some of the baud rate up to 38400 bps to
  communicate with the standard serial device.

* SMI Voyager GX uses 8 MHz baud rate generator clock which is generated
  from dividing 96 MHz PLL 0 by 12.

* This table below provide the calculation of the actual baud rate generated
  by SMI Serial Device using the following formula:

	Desired Baud Rate = fClock / (16 * Divisor Value)

  where:
	fClock = 8 Mhz (default SMI Voyager GX baud rate generator clock)
	Divisor Value = integer number between 2 to 65535.

  NOTE: Unlike the original serial device, division by 1 generates a BAUD
        signal that is constantly high.


   ==========================================================================
   | Desired Baud Rate	| Divisor Value	| Actual Baud Rate |   Difference   |
   |       (bps)	|		|  to the nearest  | to the nearest |
   |			|		|     integer	   |    integer	    |
   |			|		|      (bps)	   |      (bps)	    |
   ==========================================================================
   |          50	|    10000	|	  50	   |	    -	    |
   |          75	|     6667	|	  75	   |	    -	    |
   |         110	|     4545      |        110	   |	    -	    |
   |         135	|     3713	|        135	   |	    -	    |
   |         150	|     3333	|        150	   |	    -	    |
   |         300	|     1667	|        300	   |	    -	    |
   |         600	|      833	|        600	   |	    -	    |
   |        1200	|      417	|       1199	   |	    1	    |
   |        1800	|      277	|       1805	   |	    5	    |
   |        2000	|      250	|       2000	   |	    -	    |
   |        2400	|      208	|       2403	   |	    3	    |
   |        3600	|      139	|       3597	   |	    3	    |
   |        4800	|      104	|       4808	   |	    8	    |
   |        7200	|       69	|       7246	   |	   46	    |
   |        9600	|       52	|       9615	   |	   15	    |
   |       12800	|       39	|      12821	   |	   21	    |
   |       14400	|       35	|      14286	   |	  114	    |
   |       19200	|       26	|      19231	   |	   31	    |
   |       23040	|       22	|      22727	   |	  313	    |
   |       28800	|       17	|      29412	   |	  612	    |
   |       38400	|       13	|      38462	   |	   62	    |
   |       57600	|        9      |      55556	   |	 2044	    |
   |      115200	|        4	|     125000	   |	 9800	    |
   |      128000	|        4	|     125000	   |	 3000	    |
   |      256000	|        2	|     250000       |	 6000	    |
   ==========================================================================

* There is an alternative solution to fix this problem, i.e., by changing the
  12 Mhz or 24 Mhz crystal to other value that can support the desired baud
  rate. However, this might cause the other modules in the Voyager GX to be
  unusable, such as USB.

  Formula to find the serial fClock:

		         Crystal Value * 4
		fClock = -----------------   where 4 and 12 are fixed values.
		                12

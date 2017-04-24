/*****************************************************************************\
 *                                                                           *
 * Silicon Motion VoyagerGX SM501 register definitions.                      *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify it   *
 * under the terms of the GNU General Public License as published by the     *
 * Free Software Foundation; either version 2 of the License, or             *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful, but       *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                      *
 * See the GNU General Public License for more details.                      *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,   *
 * 675 Mass Ave, Cambridge, MA 02139, USA.                                   *
 *                                                                           *
\*****************************************************************************/

#ifndef _SM501_H_
#define _SM501_H_

/*****************************************************************************\
 *                                FIELD MACROS                               *
\*****************************************************************************/

#define _LSB(f)             (0 ? f)
#define _MSB(f)             (1 ? f)
#define _COUNT(f)           (_MSB(f) - _LSB(f) + 1)

#define RAW_MASK(f)         (0xFFFFFFFF >> (32 - _COUNT(f)))
#define GET_MASK(f)         (RAW_MASK(f) << _LSB(f))
#define GET_FIELD(d,f)      (((d) >> _LSB(f)) & RAW_MASK(f))
#define TEST_FIELD(d,f,v)   (GET_FIELD(d,f) == f ## _ ## v)
#define SET_FIELD(d,f,v)    (((d) & ~GET_MASK(f)) | \
                            (((f ## _ ## v) & RAW_MASK(f)) << _LSB(f)))
#define SET_FIELDV(d,f,v)   (((d) & ~GET_MASK(f)) | \
                            (((v) & RAW_MASK(f)) << _LSB(f)))

/*****************************************************************************\
 *                            SYSTEM CONFIGURATION                           *
\*****************************************************************************/

/* System Control */
#define SYSTEM_CONTROL                                              0x000000
#define SYSTEM_CONTROL_DPMS                                         31:30
#define SYSTEM_CONTROL_DPMS_VPHP                                    0
#define SYSTEM_CONTROL_DPMS_VPHN                                    1
#define SYSTEM_CONTROL_DPMS_VNHP                                    2
#define SYSTEM_CONTROL_DPMS_VNHN                                    3
#define SYSTEM_CONTROL_PCI_BURST                                    29:29
#define SYSTEM_CONTROL_PCI_BURST_DISABLE                            0
#define SYSTEM_CONTROL_PCI_BURST_ENABLE                             1
#define SYSTEM_CONTROL_CSC_STATUS                                   28:28
#define SYSTEM_CONTROL_CSC_STATUS_IDLE                              0
#define SYSTEM_CONTROL_CSC_STATUS_BUSY                              1
#define SYSTEM_CONTROL_PCI_BUS_MASTER                               25:25
#define SYSTEM_CONTROL_PCI_BUS_MASTER_STOP                          0
#define SYSTEM_CONTROL_PCI_BUS_MASTER_START                         1
#define SYSTEM_CONTROL_PCI_LATENCY_TIMER                            24:24
#define SYSTEM_CONTROL_PCI_LATENCY_TIMER_ENABLE                     0
#define SYSTEM_CONTROL_PCI_LATENCY_TIMER_DISABLE                    1
#define SYSTEM_CONTROL_PANEL_STATUS                                 23:23
#define SYSTEM_CONTROL_PANEL_STATUS_NORMAL                          0
#define SYSTEM_CONTROL_PANEL_STATUS_FLIP_PENDING                    1
#define SYSTEM_CONTROL_VIDEO_STATUS                                 22:22
#define SYSTEM_CONTROL_VIDEO_STATUS_NORMAL                          0
#define SYSTEM_CONTROL_VIDEO_STATUS_FLIP_PENDING                    1
#define SYSTEM_CONTROL_2D_FIFO                                      20:20
#define SYSTEM_CONTROL_2D_FIFO_NOT_EMPTY                            0
#define SYSTEM_CONTROL_2D_FIFO_EMPTY                                1
#define SYSTEM_CONTROL_2D_STATUS                                    19:19
#define SYSTEM_CONTROL_2D_STATUS_IDLE                               0
#define SYSTEM_CONTROL_2D_STATUS_BUSY                               1
#define SYSTEM_CONTROL_CRT_STATUS                                   17:17
#define SYSTEM_CONTROL_CRT_STATUS_NORMAL                            0
#define SYSTEM_CONTROL_CRT_STATUS_FLIP_PENDING                      1
#define SYSTEM_CONTROL_ZVPORT                                       16:16
#define SYSTEM_CONTROL_ZVPORT_NORMAL                                0
#define SYSTEM_CONTROL_ZVPORT_VERTICAL_SYNC_DETECTED                1
#define SYSTEM_CONTROL_PCI_BURST_READ                               15:15
#define SYSTEM_CONTROL_PCI_BURST_READ_DISABLE                       0
#define SYSTEM_CONTROL_PCI_BURST_READ_ENABLE                        1
#define SYSTEM_CONTROL_2D_ABORT                                     13:12
#define SYSTEM_CONTROL_2D_ABORT_NORMAL                              0
#define SYSTEM_CONTROL_2D_ABORT_ABORT                               3
#define SYSTEM_CONTROL_LOCK_PCI_SUBSYSTEM                           11:11
#define SYSTEM_CONTROL_LOCK_PCI_SUBSYSTEM_DISABLE                   0
#define SYSTEM_CONTROL_LOCK_PCI_SUBSYSTEM_ENABLE                    1
#define SYSTEM_CONTROL_PCI_RETRY                                    7:7
#define SYSTEM_CONTROL_PCI_RETRY_ENABLE                             0
#define SYSTEM_CONTROL_PCI_RETRY_DISABLE                            1
#define SYSTEM_CONTROL_PCI_CLOCK_RUN                                6:6
#define SYSTEM_CONTROL_PCI_CLOCK_RUN_DISABLE                        0
#define SYSTEM_CONTROL_PCI_CLOCK_RUN_ENABLE                         1
#define SYSTEM_CONTROL_PCI_SLAVE_BURST_READ_SIZE                    5:4
#define SYSTEM_CONTROL_PCI_SLAVE_BURST_READ_SIZE_1                  0
#define SYSTEM_CONTROL_PCI_SLAVE_BURST_READ_SIZE_2                  1
#define SYSTEM_CONTROL_PCI_SLAVE_BURST_READ_SIZE_4                  2
#define SYSTEM_CONTROL_PCI_SLAVE_BURST_READ_SIZE_8                  3
#define SYSTEM_CONTROL_CRT_INTERFACE                                2:2
#define SYSTEM_CONTROL_CRT_INTERFACE_NORMAL                         0
#define SYSTEM_CONTROL_CRT_INTERFACE_TRISTATE                       1
#define SYSTEM_CONTROL_LOCAL_MEMORY_INTERFACE                       1:1
#define SYSTEM_CONTROL_LOCAL_MEMORY_INTERFACE_NORMAL                0
#define SYSTEM_CONTROL_LOCAL_MEMORY_INTERFACE_TRISTATE              1
#define SYSTEM_CONTROL_PANEL_INTERFACE                              0:0
#define SYSTEM_CONTROL_PANEL_INTERFACE_NORMAL                       0
#define SYSTEM_CONTROL_PANEL_INTERFACE_TRISTATE                     1

/* Miscellaneous Control */
#define MISCELLANEOUS_CONTROL                                       0x000004
#define MISCELLANEOUS_CONTROL_PCI_PAD_DRIVE                         31:30
#define MISCELLANEOUS_CONTROL_PCI_PAD_DRIVE_24MA                    0
#define MISCELLANEOUS_CONTROL_PCI_PAD_DRIVE_12MA                    1
#define MISCELLANEOUS_CONTROL_PCI_PAD_DRIVE_8MA                     2
#define MISCELLANEOUS_CONTROL_USB_CLOCK_SELECT                      29:28
#define MISCELLANEOUS_CONTROL_USB_CLOCK_SELECT_CRYSTAL              0
#define MISCELLANEOUS_CONTROL_USB_CLOCK_SELECT_HOST_96              2
#define MISCELLANEOUS_CONTROL_USB_CLOCK_SELECT_HOST_48              3
#define MISCELLANEOUS_CONTROL_UART1_SELECT                          27:27
#define MISCELLANEOUS_CONTROL_UART1_SELECT_UART                     0
#define MISCELLANEOUS_CONTROL_UART1_SELECT_SSP                      1
#define MISCELLANEOUS_CONTROL_8051_LATCH                            26:26
#define MISCELLANEOUS_CONTROL_8051_LATCH_DISABLE                    0
#define MISCELLANEOUS_CONTROL_8051_LATCH_ENABLE                     1
#define MISCELLANEOUS_CONTROL_FLAT_PANEL                            25:25
#define MISCELLANEOUS_CONTROL_FLAT_PANEL_18BIT                      0
#define MISCELLANEOUS_CONTROL_FLAT_PANEL_24BIT                      1
#define MISCELLANEOUS_CONTROL_CRYSTAL                               24:24
#define MISCELLANEOUS_CONTROL_CRYSTAL_24MHZ                         0
#define MISCELLANEOUS_CONTROL_CRYSTAL_12MHZ                         1
#define MISCELLANEOUS_CONTROL_DRAM_REFRESH                          22:21
#define MISCELLANEOUS_CONTROL_DRAM_REFRESH_8US                      0
#define MISCELLANEOUS_CONTROL_DRAM_REFRESH_16US                     1
#define MISCELLANEOUS_CONTROL_DRAM_REFRESH_32US                     2
#define MISCELLANEOUS_CONTROL_DRAM_REFRESH_64US                     3
#define MISCELLANEOUS_CONTROL_BUS_HOLD                              20:18
#define MISCELLANEOUS_CONTROL_BUS_HOLD_FIFO_EMPTY                   0
#define MISCELLANEOUS_CONTROL_BUS_HOLD_8_TRANSACTIONS               1
#define MISCELLANEOUS_CONTROL_BUS_HOLD_16_TRANSACTIONS              2
#define MISCELLANEOUS_CONTROL_BUS_HOLD_24_TRANSACTIONS              3
#define MISCELLANEOUS_CONTROL_BUS_HOLD_32_TRANSACTIONS              4
#define MISCELLANEOUS_CONTROL_HITACHI_READY                         17:17
#define MISCELLANEOUS_CONTROL_HITACHI_READY_ACTIVE_LOW              0
#define MISCELLANEOUS_CONTROL_HITACHI_READY_ACTIVE_HIGH             1
#define MISCELLANEOUS_CONTROL_INTERRUPT                             16:16
#define MISCELLANEOUS_CONTROL_INTERRUPT_NORMAL                      0
#define MISCELLANEOUS_CONTROL_INTERRUPT_INVERTED                    1
#define MISCELLANEOUS_CONTROL_PLL_CLOCK_COUNT                       15:15
#define MISCELLANEOUS_CONTROL_PLL_CLOCK_COUNT_DISABLE               0
#define MISCELLANEOUS_CONTROL_PLL_CLOCK_COUNT_ENABLE                1
#define MISCELLANEOUS_CONTROL_DAC_BAND_GAP                          14:13
#define MISCELLANEOUS_CONTROL_DAC_BAND_GAP_NORMAL                   0
#define MISCELLANEOUS_CONTROL_DAC_POWER                             12:12
#define MISCELLANEOUS_CONTROL_DAC_POWER_ENABLE                      0
#define MISCELLANEOUS_CONTROL_DAC_POWER_DISABLE                     1
#define MISCELLANEOUS_CONTROL_USB_SLAVE                             11:11
#define MISCELLANEOUS_CONTROL_USB_SLAVE_CPU                         0
#define MISCELLANEOUS_CONTROL_USB_SLAVE_8051                        1
#define MISCELLANEOUS_CONTROL_CPU_MASTER_BURST                      10:10
#define MISCELLANEOUS_CONTROL_CPU_MASTER_BURST_8                    0
#define MISCELLANEOUS_CONTROL_CPU_MASTER_BURST_1                    1
#define MISCELLANEOUS_CONTROL_USB_PORT_SELECT                       9:9
#define MISCELLANEOUS_CONTROL_USB_PORT_SELECT_MASTER                0
#define MISCELLANEOUS_CONTROL_USB_PORT_SELECT_SLAVE                 1
#define MISCELLANEOUS_CONTROL_USB_LOOPBACK                          8:8
#define MISCELLANEOUS_CONTROL_USB_LOOPBACK_NORMAL                   0
#define MISCELLANEOUS_CONTROL_USB_LOOPBACK_INTERNAL                 1
#define MISCELLANEOUS_CONTROL_CLOCK_DIVIDER                         7:7
#define MISCELLANEOUS_CONTROL_CLOCK_DIVIDER_NORMAL                  0
#define MISCELLANEOUS_CONTROL_CLOCK_DIVIDER_RESET                   1
#define MISCELLANEOUS_CONTROL_TEST_MODE                             6:5
#define MISCELLANEOUS_CONTROL_TEST_MODE_NORMAL                      0
#define MISCELLANEOUS_CONTROL_TEST_MODE_DEBUGGING                   1
#define MISCELLANEOUS_CONTROL_TEST_MODE_NAND_TREE                   2
#define MISCELLANEOUS_CONTROL_TEST_MODE_MEMORY                      3
#define MISCELLANEOUS_CONTROL_NEC_MEMORY_MAP                        4:4
#define MISCELLANEOUS_CONTROL_NEC_MEMORY_MAP_30MB                   0
#define MISCELLANEOUS_CONTROL_NEC_MEMORY_MAP_62MB                   1
#define MISCELLANEOUS_CONTROL_CLOCK                                 3:3
#define MISCELLANEOUS_CONTROL_CLOCK_PLL                             0
#define MISCELLANEOUS_CONTROL_CLOCK_TEST_CLOCK                      1
#define MISCELLANEOUS_CONTROL_HOST_BUS                              2:0
#define MISCELLANEOUS_CONTROL_HOST_BUS_HITACHI                      0
#define MISCELLANEOUS_CONTROL_HOST_BUS_PCI                          1
#define MISCELLANEOUS_CONTROL_HOST_BUS_INTEL                        2
#define MISCELLANEOUS_CONTROL_HOST_BUS_NEC                          6

/* GPIO[31:0] Control */
#define GPIO_CONTROL_LOW                                            0x000008
#define GPIO_CONTROL_LOW_31                                         31:31
#define GPIO_CONTROL_LOW_31_GPIO                                    0
#define GPIO_CONTROL_LOW_31_PWM2                                    1
#define GPIO_CONTROL_LOW_30                                         30:30
#define GPIO_CONTROL_LOW_30_GPIO                                    0
#define GPIO_CONTROL_LOW_30_PWM1                                    1
#define GPIO_CONTROL_LOW_29                                         29:29
#define GPIO_CONTROL_LOW_29_GPIO                                    0
#define GPIO_CONTROL_LOW_29_PWM0                                    1
#define GPIO_CONTROL_LOW_28                                         28:28
#define GPIO_CONTROL_LOW_28_GPIO                                    0
#define GPIO_CONTROL_LOW_28_AC97_RX_I2S                             1
#define GPIO_CONTROL_LOW_27                                         27:27
#define GPIO_CONTROL_LOW_27_GPIO                                    0
#define GPIO_CONTROL_LOW_27_AC97_TX_I2S                             1
#define GPIO_CONTROL_LOW_26                                         26:26
#define GPIO_CONTROL_LOW_26_GPIO                                    0
#define GPIO_CONTROL_LOW_26_AC97_CLOCK_I2S                          1
#define GPIO_CONTROL_LOW_25                                         25:25
#define GPIO_CONTROL_LOW_25_GPIO                                    0
#define GPIO_CONTROL_LOW_25_AC97_SYNC_I2S                           1
#define GPIO_CONTROL_LOW_24                                         24:24
#define GPIO_CONTROL_LOW_24_GPIO                                    0
#define GPIO_CONTROL_LOW_24_AC97_RESET                              1
#define GPIO_CONTROL_LOW_23                                         23:23
#define GPIO_CONTROL_LOW_23_GPIO                                    0
#define GPIO_CONTROL_LOW_23_ZVPORT7                                 1
#define GPIO_CONTROL_LOW_22                                         22:22
#define GPIO_CONTROL_LOW_22_GPIO                                    0
#define GPIO_CONTROL_LOW_22_ZVPORT6                                 1
#define GPIO_CONTROL_LOW_21                                         21:21
#define GPIO_CONTROL_LOW_21_GPIO                                    0
#define GPIO_CONTROL_LOW_21_ZVPORT5                                 1
#define GPIO_CONTROL_LOW_20                                         20:20
#define GPIO_CONTROL_LOW_20_GPIO                                    0
#define GPIO_CONTROL_LOW_20_ZVPORT4                                 1
#define GPIO_CONTROL_LOW_19                                         19:19
#define GPIO_CONTROL_LOW_19_GPIO                                    0
#define GPIO_CONTROL_LOW_19_ZVPORT3                                 1
#define GPIO_CONTROL_LOW_18                                         18:18
#define GPIO_CONTROL_LOW_18_GPIO                                    0
#define GPIO_CONTROL_LOW_18_ZVPORT2                                 1
#define GPIO_CONTROL_LOW_17                                         17:17
#define GPIO_CONTROL_LOW_17_GPIO                                    0
#define GPIO_CONTROL_LOW_17_ZVPORT1                                 1
#define GPIO_CONTROL_LOW_16                                         16:16
#define GPIO_CONTROL_LOW_16_GPIO                                    0
#define GPIO_CONTROL_LOW_16_ZVPORT0                                 1
#define GPIO_CONTROL_LOW_15                                         15:15
#define GPIO_CONTROL_LOW_15_GPIO                                    0
#define GPIO_CONTROL_LOW_15_8051_A11                                1
#define GPIO_CONTROL_LOW_14                                         14:14
#define GPIO_CONTROL_LOW_14_GPIO                                    0
#define GPIO_CONTROL_LOW_14_8051_A10                                1
#define GPIO_CONTROL_LOW_13                                         13:13
#define GPIO_CONTROL_LOW_13_GPIO                                    0
#define GPIO_CONTROL_LOW_13_8051_A9                                 1
#define GPIO_CONTROL_LOW_12                                         12:12
#define GPIO_CONTROL_LOW_12_GPIO                                    0
#define GPIO_CONTROL_LOW_12_8051_A8                                 1
#define GPIO_CONTROL_LOW_11                                         11:11
#define GPIO_CONTROL_LOW_11_GPIO                                    0
#define GPIO_CONTROL_LOW_11_8051_WAIT                               1
#define GPIO_CONTROL_LOW_10                                         10:10
#define GPIO_CONTROL_LOW_10_GPIO                                    0
#define GPIO_CONTROL_LOW_10_8051_ALE                                1
#define GPIO_CONTROL_LOW_9                                          9:9
#define GPIO_CONTROL_LOW_9_GPIO                                     0
#define GPIO_CONTROL_LOW_9_8051_WRITE                               1
#define GPIO_CONTROL_LOW_8                                          8:8
#define GPIO_CONTROL_LOW_8_GPIO                                     0
#define GPIO_CONTROL_LOW_8_8051_READ                                1
#define GPIO_CONTROL_LOW_7                                          7:7
#define GPIO_CONTROL_LOW_7_GPIO                                     0
#define GPIO_CONTROL_LOW_7_8051_AD7                                 1
#define GPIO_CONTROL_LOW_6                                          6:6
#define GPIO_CONTROL_LOW_6_GPIO                                     0
#define GPIO_CONTROL_LOW_6_8051_AD6                                 1
#define GPIO_CONTROL_LOW_5                                          5:5
#define GPIO_CONTROL_LOW_5_GPIO                                     0
#define GPIO_CONTROL_LOW_5_8051_AD5                                 1
#define GPIO_CONTROL_LOW_4                                          4:4
#define GPIO_CONTROL_LOW_4_GPIO                                     0
#define GPIO_CONTROL_LOW_4_8051_AD4                                 1
#define GPIO_CONTROL_LOW_3                                          3:3
#define GPIO_CONTROL_LOW_3_GPIO                                     0
#define GPIO_CONTROL_LOW_3_8051_AD3                                 1
#define GPIO_CONTROL_LOW_2                                          2:2
#define GPIO_CONTROL_LOW_2_GPIO                                     0
#define GPIO_CONTROL_LOW_2_8051_AD2                                 1
#define GPIO_CONTROL_LOW_1                                          1:1
#define GPIO_CONTROL_LOW_1_GPIO                                     0
#define GPIO_CONTROL_LOW_1_8051_AD1                                 1
#define GPIO_CONTROL_LOW_0                                          0:0
#define GPIO_CONTROL_LOW_0_GPIO                                     0
#define GPIO_CONTROL_LOW_0_8051_AD0                                 1

/* GPIO[63:32] Control */
#define GPIO_CONTROL_HIGH                                           0x00000C
#define GPIO_CONTROL_HIGH_63                                        31:31
#define GPIO_CONTROL_HIGH_63_GPIO                                   0
#define GPIO_CONTROL_HIGH_63_CRT7_ZVPORT15                          1
#define GPIO_CONTROL_HIGH_62                                        30:30
#define GPIO_CONTROL_HIGH_62_GPIO                                   0
#define GPIO_CONTROL_HIGH_62_CRT6_ZVPORT14                          1
#define GPIO_CONTROL_HIGH_61                                        29:29
#define GPIO_CONTROL_HIGH_61_GPIO                                   0
#define GPIO_CONTROL_HIGH_61_CRT5_ZVPORT13_PANEL17                  1
#define GPIO_CONTROL_HIGH_60                                        28:28
#define GPIO_CONTROL_HIGH_60_GPIO                                   0
#define GPIO_CONTROL_HIGH_60_CRT4_ZVPORT12_PANEL16                  1
#define GPIO_CONTROL_HIGH_59                                        27:27
#define GPIO_CONTROL_HIGH_59_GPIO                                   0
#define GPIO_CONTROL_HIGH_59_CRT3_ZVPORT11_PANEL9                   1
#define GPIO_CONTROL_HIGH_58                                        26:26
#define GPIO_CONTROL_HIGH_58_GPIO                                   0
#define GPIO_CONTROL_HIGH_58_CRT2_ZVPORT10_PANEL8                   1
#define GPIO_CONTROL_HIGH_57                                        25:25
#define GPIO_CONTROL_HIGH_57_GPIO                                   0
#define GPIO_CONTROL_HIGH_57_CRT1_ZVPORT9_PANEL1                    1
#define GPIO_CONTROL_HIGH_56                                        24:24
#define GPIO_CONTROL_HIGH_56_GPIO                                   0
#define GPIO_CONTROL_HIGH_56_CRT0_ZVPORT8_PANEL0                    1
#define GPIO_CONTROL_HIGH_55                                        23:23
#define GPIO_CONTROL_HIGH_55_GPIO                                   0
#define GPIO_CONTROL_HIGH_55_CRT_CLOCK                              1
#define GPIO_CONTROL_HIGH_47                                        15:15
#define GPIO_CONTROL_HIGH_47_GPIO                                   0
#define GPIO_CONTROL_HIGH_47_I2C_DATA                               1
#define GPIO_CONTROL_HIGH_46                                        14:14
#define GPIO_CONTROL_HIGH_46_GPIO                                   0
#define GPIO_CONTROL_HIGH_46_I2C_CLOCK                              1
#define GPIO_CONTROL_HIGH_45                                        13:13
#define GPIO_CONTROL_HIGH_45_GPIO                                   0
#define GPIO_CONTROL_HIGH_45_SSP1_CLOCK                             1
#define GPIO_CONTROL_HIGH_44                                        12:12
#define GPIO_CONTROL_HIGH_44_GPIO                                   0
#define GPIO_CONTROL_HIGH_44_UART1_RTS_SSP1_OUT                     1
#define GPIO_CONTROL_HIGH_43                                        11:11
#define GPIO_CONTROL_HIGH_43_GPIO                                   0
#define GPIO_CONTROL_HIGH_43_UART1_CTS_SSP1_IN                      1
#define GPIO_CONTROL_HIGH_42                                        10:10
#define GPIO_CONTROL_HIGH_42_GPIO                                   0
#define GPIO_CONTROL_HIGH_42_UART1_RX_SSP1_RX                       1
#define GPIO_CONTROL_HIGH_41                                        9:9
#define GPIO_CONTROL_HIGH_41_GPIO                                   0
#define GPIO_CONTROL_HIGH_41_UART1_TX_SSP1_TX                       1
#define GPIO_CONTROL_HIGH_40                                        8:8
#define GPIO_CONTROL_HIGH_40_GPIO                                   0
#define GPIO_CONTROL_HIGH_40_UART0_RTS                              1
#define GPIO_CONTROL_HIGH_39                                        7:7
#define GPIO_CONTROL_HIGH_39_GPIO                                   0
#define GPIO_CONTROL_HIGH_39_UART0_CTS                              1
#define GPIO_CONTROL_HIGH_38                                        6:6
#define GPIO_CONTROL_HIGH_38_GPIO                                   0
#define GPIO_CONTROL_HIGH_38_UART0_RX                               1
#define GPIO_CONTROL_HIGH_37                                        5:5
#define GPIO_CONTROL_HIGH_37_GPIO                                   0
#define GPIO_CONTROL_HIGH_37_UART0_TX                               1
#define GPIO_CONTROL_HIGH_36                                        4:4
#define GPIO_CONTROL_HIGH_36_GPIO                                   0
#define GPIO_CONTROL_HIGH_36_SSP0_CLOCK                             1
#define GPIO_CONTROL_HIGH_35                                        3:3
#define GPIO_CONTROL_HIGH_35_GPIO                                   0
#define GPIO_CONTROL_HIGH_35_SSP0_IN                                1
#define GPIO_CONTROL_HIGH_34                                        2:2
#define GPIO_CONTROL_HIGH_34_GPIO                                   0
#define GPIO_CONTROL_HIGH_34_SSP0_OUT                               1
#define GPIO_CONTROL_HIGH_33                                        1:1
#define GPIO_CONTROL_HIGH_33_GPIO                                   0
#define GPIO_CONTROL_HIGH_33_SSP0_RX                                1
#define GPIO_CONTROL_HIGH_32                                        0:0
#define GPIO_CONTROL_HIGH_32_GPIO                                   0
#define GPIO_CONTROL_HIGH_32_SSP0_TX                                1

/* DRAM Control */
#define DRAM_CONTROL                                                0x000010
#define DRAM_CONTROL_EMBEDDED_MEMORY                                31:31
#define DRAM_CONTROL_EMBEDDED_MEMORY_ENABLE                         0
#define DRAM_CONTROL_EMBEDDED_MEMORY_DISABLE                        1
#define DRAM_CONTROL_EXTERNAL_BURST                                 30:28
#define DRAM_CONTROL_EXTERNAL_BURST_1                               0
#define DRAM_CONTROL_EXTERNAL_BURST_2                               1
#define DRAM_CONTROL_EXTERNAL_BURST_4                               2
#define DRAM_CONTROL_EXTERNAL_BURST_8                               3
#define DRAM_CONTROL_EXTERNAL_CAS_LATENCY                           27:27
#define DRAM_CONTROL_EXTERNAL_CAS_LATENCY_2                         0
#define DRAM_CONTROL_EXTERNAL_CAS_LATENCY_3                         1
#define DRAM_CONTROL_EXTERNAL_SIZE                                  26:24
#define DRAM_CONTROL_EXTERNAL_SIZE_2MB                              0
#define DRAM_CONTROL_EXTERNAL_SIZE_4MB                              1
#define DRAM_CONTROL_EXTERNAL_SIZE_64MB                             4
#define DRAM_CONTROL_EXTERNAL_SIZE_32MB                             5
#define DRAM_CONTROL_EXTERNAL_SIZE_16MB                             6
#define DRAM_CONTROL_EXTERNAL_SIZE_8MB                              7
#define DRAM_CONTROL_EXTERNAL_COLUMN_SIZE                           23:22
#define DRAM_CONTROL_EXTERNAL_COLUMN_SIZE_1024                      0
#define DRAM_CONTROL_EXTERNAL_COLUMN_SIZE_512                       2
#define DRAM_CONTROL_EXTERNAL_COLUMN_SIZE_256                       3
#define DRAM_CONTROL_EXTERNAL_ACTIVE_PRECHARGE                      21:21
#define DRAM_CONTROL_EXTERNAL_ACTIVE_PRECHARGE_6                    0
#define DRAM_CONTROL_EXTERNAL_ACTIVE_PRECHARGE_7                    1
#define DRAM_CONTROL_EXTERNAL                                       20:20
#define DRAM_CONTROL_EXTERNAL_RESET                                 0
#define DRAM_CONTROL_EXTERNAL_NORMAL                                1
#define DRAM_CONTROL_EXTERNAL_BANKS                                 19:19
#define DRAM_CONTROL_EXTERNAL_BANKS_2                               0
#define DRAM_CONTROL_EXTERNAL_BANKS_4                               1
#define DRAM_CONTROL_EXTERNAL_WRITE_PRECHARGE                       18:18
#define DRAM_CONTROL_EXTERNAL_WRITE_PRECHARGE_2                     0
#define DRAM_CONTROL_EXTERNAL_WRITE_PRECHARGE_1                     1
#define DRAM_CONTROL_LOCAL_BLOCK_WRITE                              17:17
#define DRAM_CONTROL_LOCAL_BLOCK_WRITE_DISABLE                      0
#define DRAM_CONTROL_LOCAL_BLOCK_WRITE_ENABLE                       1
#define DRAM_CONTROL_LOCAL_REFRESH_COMMAND                          16:16
#define DRAM_CONTROL_LOCAL_REFRESH_COMMAND_10                       0
#define DRAM_CONTROL_LOCAL_REFRESH_COMMAND_12                       1
#define DRAM_CONTROL_LOCAL_SIZE                                     15:13
#define DRAM_CONTROL_LOCAL_SIZE_4MB                                 0
#define DRAM_CONTROL_LOCAL_SIZE_8MB                                 1
#define DRAM_CONTROL_LOCAL_SIZE_16MB                                2
#define DRAM_CONTROL_LOCAL_SIZE_32MB                                3
#define DRAM_CONTROL_LOCAL_SIZE_64MB                                4
#define DRAM_CONTROL_LOCAL_COLUMN_SIZE                              12:11
#define DRAM_CONTROL_LOCAL_COLUMN_SIZE_256                          0
#define DRAM_CONTROL_LOCAL_COLUMN_SIZE_512                          2
#define DRAM_CONTROL_LOCAL_COLUMN_SIZE_1024                         3
#define DRAM_CONTROL_LOCAL_BLOCK_WRITE_TIME                         10:10
#define DRAM_CONTROL_LOCAL_BLOCK_WRITE_TIME_1                       0
#define DRAM_CONTROL_LOCAL_BLOCK_WRITE_TIME_2                       1
#define DRAM_CONTROL_LOCAL_BLOCK_WRITE_PRECHARGE                    9:9
#define DRAM_CONTROL_LOCAL_BLOCK_WRITE_PRECHARGE_4                  0
#define DRAM_CONTROL_LOCAL_BLOCK_WRITE_PRECHARGE_1                  1
#define DRAM_CONTROL_LOCAL_ACTIVE_PRECHARGE                         8:8
#define DRAM_CONTROL_LOCAL_ACTIVE_PRECHARGE_6                       0
#define DRAM_CONTROL_LOCAL_ACTIVE_PRECHARGE_7                       1
#define DRAM_CONTROL_LOCAL                                          7:7
#define DRAM_CONTROL_LOCAL_NORMAL                                   0
#define DRAM_CONTROL_LOCAL_RESET                                    1
#define DRAM_CONTROL_LOCAL_REMAIN_ACTIVE                            6:6
#define DRAM_CONTROL_LOCAL_REMAIN_ACTIVE_ENABLE                     0
#define DRAM_CONTROL_LOCAL_REMAIN_ACTIVE_DISABLE                    1
#define DRAM_CONTROL_LOCAL_BANKS                                    1:1
#define DRAM_CONTROL_LOCAL_BANKS_4                                  0
#define DRAM_CONTROL_LOCAL_BANKS_2                                  1
#define DRAM_CONTROL_LOCAL_WRITE_PRECHARGE                          0:0
#define DRAM_CONTROL_LOCAL_WRITE_PRECHARGE_2                        0
#define DRAM_CONTROL_LOCAL_WRITE_PRECHARGE_1                        1

/* Arbitration Control */
#define ARBITRATION_CONTROL                                         0x000014
#define ARBITRATION_CONTROL_EXTERNAL_MEMORY                         29:29
#define ARBITRATION_CONTROL_EXTERNAL_MEMORY_FIXED                   0
#define ARBITRATION_CONTROL_EXTERNAL_MEMORY_REVOLVING               1
#define ARBITRATION_CONTROL_LOCAL_MEMORY                            28:28
#define ARBITRATION_CONTROL_LOCAL_MEMORY_FIXED                      0
#define ARBITRATION_CONTROL_LOCAL_MEMORY_REVOLVING                  1
#define ARBITRATION_CONTROL_USB                                     26:24
#define ARBITRATION_CONTROL_USB_OFF                                 0
#define ARBITRATION_CONTROL_USB_PRIORITY_1                          1
#define ARBITRATION_CONTROL_USB_PRIORITY_2                          2
#define ARBITRATION_CONTROL_USB_PRIORITY_3                          3
#define ARBITRATION_CONTROL_USB_PRIORITY_4                          4
#define ARBITRATION_CONTROL_USB_PRIORITY_5                          5
#define ARBITRATION_CONTROL_USB_PRIORITY_6                          6
#define ARBITRATION_CONTROL_USB_PRIORITY_7                          7
#define ARBITRATION_CONTROL_PANEL                                   22:20
#define ARBITRATION_CONTROL_PANEL_OFF                               0
#define ARBITRATION_CONTROL_PANEL_PRIORITY_1                        1
#define ARBITRATION_CONTROL_PANEL_PRIORITY_2                        2
#define ARBITRATION_CONTROL_PANEL_PRIORITY_3                        3
#define ARBITRATION_CONTROL_PANEL_PRIORITY_4                        4
#define ARBITRATION_CONTROL_PANEL_PRIORITY_5                        5
#define ARBITRATION_CONTROL_PANEL_PRIORITY_6                        6
#define ARBITRATION_CONTROL_PANEL_PRIORITY_7                        7
#define ARBITRATION_CONTROL_ZVPORT                                  18:16
#define ARBITRATION_CONTROL_ZVPORT_OFF                              0
#define ARBITRATION_CONTROL_ZVPORT_PRIORITY_1                       1
#define ARBITRATION_CONTROL_ZVPORT_PRIORITY_2                       2
#define ARBITRATION_CONTROL_ZVPORT_PRIORITY_3                       3
#define ARBITRATION_CONTROL_ZVPORT_PRIORITY_4                       4
#define ARBITRATION_CONTROL_ZVPORT_PRIORITY_5                       5
#define ARBITRATION_CONTROL_ZVPORT_PRIORITY_6                       6
#define ARBITRATION_CONTROL_ZVPORT_PRIORITY_7                       7
#define ARBITRATION_CONTROL_COMMAND_LIST                            14:12
#define ARBITRATION_CONTROL_COMMAND_LIST_OFF                        0
#define ARBITRATION_CONTROL_COMMAND_LIST_PRIORITY_1                 1
#define ARBITRATION_CONTROL_COMMAND_LIST_PRIORITY_2                 2
#define ARBITRATION_CONTROL_COMMAND_LIST_PRIORITY_3                 3
#define ARBITRATION_CONTROL_COMMAND_LIST_PRIORITY_4                 4
#define ARBITRATION_CONTROL_COMMAND_LIST_PRIORITY_5                 5
#define ARBITRATION_CONTROL_COMMAND_LIST_PRIORITY_6                 6
#define ARBITRATION_CONTROL_COMMAND_LIST_PRIORITY_7                 7
#define ARBITRATION_CONTROL_DMA                                     10:8
#define ARBITRATION_CONTROL_DMA_OFF                                 0
#define ARBITRATION_CONTROL_DMA_PRIORITY_1                          1
#define ARBITRATION_CONTROL_DMA_PRIORITY_2                          2
#define ARBITRATION_CONTROL_DMA_PRIORITY_3                          3
#define ARBITRATION_CONTROL_DMA_PRIORITY_4                          4
#define ARBITRATION_CONTROL_DMA_PRIORITY_5                          5
#define ARBITRATION_CONTROL_DMA_PRIORITY_6                          6
#define ARBITRATION_CONTROL_DMA_PRIORITY_7                          7
#define ARBITRATION_CONTROL_VIDEO                                   6:4
#define ARBITRATION_CONTROL_VIDEO_OFF                               0
#define ARBITRATION_CONTROL_VIDEO_PRIORITY_1                        1
#define ARBITRATION_CONTROL_VIDEO_PRIORITY_2                        2
#define ARBITRATION_CONTROL_VIDEO_PRIORITY_3                        3
#define ARBITRATION_CONTROL_VIDEO_PRIORITY_4                        4
#define ARBITRATION_CONTROL_VIDEO_PRIORITY_5                        5
#define ARBITRATION_CONTROL_VIDEO_PRIORITY_6                        6
#define ARBITRATION_CONTROL_VIDEO_PRIORITY_7                        7
#define ARBITRATION_CONTROL_CRT                                     2:0
#define ARBITRATION_CONTROL_CRT_OFF                                 0
#define ARBITRATION_CONTROL_CRT_PRIORITY_1                          1
#define ARBITRATION_CONTROL_CRT_PRIORITY_2                          2
#define ARBITRATION_CONTROL_CRT_PRIORITY_3                          3
#define ARBITRATION_CONTROL_CRT_PRIORITY_4                          4
#define ARBITRATION_CONTROL_CRT_PRIORITY_5                          5
#define ARBITRATION_CONTROL_CRT_PRIORITY_6                          6
#define ARBITRATION_CONTROL_CRT_PRIORITY_7                          7

/* Command List Control */
#define COMMAND_LIST_CONTROL                                        0x000018
#define COMMAND_LIST_CONTROL_STATUS                                 31:31
#define COMMAND_LIST_CONTROL_STATUS_STOPPED                         0
#define COMMAND_LIST_CONTROL_STATUS_RUNNING                         1
#define COMMAND_LIST_CONTROL_MEMORY                                 27:26
#define COMMAND_LIST_CONTROL_MEMORY_LOCAL                           0
#define COMMAND_LIST_CONTROL_MEMORY_EXTERNAL_CS0                    2
#define COMMAND_LIST_CONTROL_MEMORY_EXTERNAL_CS1                    3
#define COMMAND_LIST_CONTROL_ADDRESS                                25:0

/* Command List Conditions */
#define COMMAND_LIST_CONDITIONS                                     0x00001C

/* Command List Return */
#define COMMAND_LIST_RETURN                                         0x000020
#define COMMAND_LIST_RETURN_MEMORY                                  27:26
#define COMMAND_LIST_RETURN_MEMORY_LOCAL                            0
#define COMMAND_LIST_RETURN_MEMORY_EXTERNAL_CS0                     2
#define COMMAND_LIST_RETURN_MEMORY_EXTERNAL_CS1                     3
#define COMMAND_LIST_RETURN_ADDRESS                                 25:0

/* Command List Status */
#define COMMAND_LIST_STATUS                                         0x000024
#define COMMAND_LIST_STATUS_2D_MEMORY_FIFO                          20:20
#define COMMAND_LIST_STATUS_2D_MEMORY_FIFO_NOT_EMPTY                0
#define COMMAND_LIST_STATUS_2D_MEMORY_FIFO_EMPTY                    1
#define COMMAND_LIST_STATUS_COMMAND_FIFO                            19:19
#define COMMAND_LIST_STATUS_COMMAND_FIFO_NOT_EMPTY                  0
#define COMMAND_LIST_STATUS_COMMAND_FIFO_EMPTY                      1
#define COMMAND_LIST_STATUS_CSC                                     18:18
#define COMMAND_LIST_STATUS_CSC_IDLE                                0
#define COMMAND_LIST_STATUS_CSC_BUSY                                1
#define COMMAND_LIST_STATUS_MEMORY_DMA                              17:17
#define COMMAND_LIST_STATUS_MEMORY_DMA_IDLE                         0
#define COMMAND_LIST_STATUS_MEMORY_DMA_BUSY                         1
#define COMMAND_LIST_STATUS_CRT_STATUS                              16:16
#define COMMAND_LIST_STATUS_CRT_STATUS_NORMAL                       0
#define COMMAND_LIST_STATUS_CRT_STATUS_FLIP_PENDING                 1
#define COMMAND_LIST_STATUS_CURRENT_FIELD                           15:15
#define COMMAND_LIST_STATUS_CURRENT_FIELD_ODD                       0
#define COMMAND_LIST_STATUS_CURRENT_FIELD_EVEN                      1
#define COMMAND_LIST_STATUS_VIDEO_STATUS                            14:14
#define COMMAND_LIST_STATUS_VIDEO_STATUS_NORMAL                     0
#define COMMAND_LIST_STATUS_VIDEO_STATUS_FLIP_PENDING               1
#define COMMAND_LIST_STATUS_PANEL_STATUS                            13:13
#define COMMAND_LIST_STATUS_PANEL_STATUS_NORMAL                     0
#define COMMAND_LIST_STATUS_PANEL_STATUS_FLIP_PENDING               1
#define COMMAND_LIST_STATUS_CRT_VERTICAL_SYNC                       12:12
#define COMMAND_LIST_STATUS_CRT_VERTICAL_SYNC_NOT_ACTIVE            0
#define COMMAND_LIST_STATUS_CRT_VERTICAL_SYNC_ACTIVE                1
#define COMMAND_LIST_STATUS_PANEL_VERTICAL_SYNC                     11:11
#define COMMAND_LIST_STATUS_PANEL_VERTICAL_SYNC_NOT_ACTIVE          0
#define COMMAND_LIST_STATUS_PANEL_VERTICAL_SYNC_ACTIVE              1
#define COMMAND_LIST_STATUS_2D_SETUP                                2:2
#define COMMAND_LIST_STATUS_2D_SETUP_IDLE                           0
#define COMMAND_LIST_STATUS_2D_SETUP_BUSY                           1
#define COMMAND_LIST_STATUS_2D_FIFO                                 1:1
#define COMMAND_LIST_STATUS_2D_FIFO_NOT_EMPTY                       0
#define COMMAND_LIST_STATUS_2D_FIFO_EMPTY                           1
#define COMMAND_LIST_STATUS_2D                                      0:0
#define COMMAND_LIST_STATUS_2D_IDLE                                 0
#define COMMAND_LIST_STATUS_2D_BUSY                                 1

/* Raw Interrupt Status & Clear */
#define RAW_INTERRUPT_STATUS                                        0x000028
#define RAW_INTERRUPT_STATUS_USB_SLAVE_PLUG_IN                      5:5
#define RAW_INTERRUPT_STATUS_USB_SLAVE_PLUG_IN_NOT_ACTIVE           0
#define RAW_INTERRUPT_STATUS_USB_SLAVE_PLUG_IN_ACTIVE               1
#define RAW_INTERRUPT_STATUS_USB_SLAVE_PLUG_IN_CLEAR                1
#define RAW_INTERRUPT_STATUS_ZVPORT                                 4:4
#define RAW_INTERRUPT_STATUS_ZVPORT_NOT_ACTIVE                      0
#define RAW_INTERRUPT_STATUS_ZVPORT_ACTIVE                          1
#define RAW_INTERRUPT_STATUS_ZVPORT_CLEAR                           1
#define RAW_INTERRUPT_STATUS_CRT_VERTICAL_SYNC                      3:3
#define RAW_INTERRUPT_STATUS_CRT_VERTICAL_SYNC_NOT_ACTIVE           0
#define RAW_INTERRUPT_STATUS_CRT_VERTICAL_SYNC_ACTIVE               1
#define RAW_INTERRUPT_STATUS_CRT_VERTICAL_SYNC_CLEAR                1
#define RAW_INTERRUPT_STATUS_USB_SLAVE                              2:2
#define RAW_INTERRUPT_STATUS_USB_SLAVE_NOT_ACTIVE                   0
#define RAW_INTERRUPT_STATUS_USB_SLAVE_ACTIVE                       1
#define RAW_INTERRUPT_STATUS_USB_SLAVE_CLEAR                        1
#define RAW_INTERRUPT_STATUS_PANEL_VERTICAL_SYNC                    1:1
#define RAW_INTERRUPT_STATUS_PANEL_VERTICAL_SYNC_NOT_ACTIVE         0
#define RAW_INTERRUPT_STATUS_PANEL_VERTICAL_SYNC_ACTIVE             1
#define RAW_INTERRUPT_STATUS_PANEL_VERTICAL_SYNC_CLEAR              1
#define RAW_INTERRUPT_STATUS_COMMAND_LIST                           0:0
#define RAW_INTERRUPT_STATUS_COMMAND_LIST_NOT_ACTIVE                0
#define RAW_INTERRUPT_STATUS_COMMAND_LIST_ACTIVE                    1
#define RAW_INTERRUPT_STATUS_COMMAND_LIST_CLEAR                     1

/* Interrupt Status */
#define INTERRUPT_STATUS                                            0x00002C
#define INTERRUPT_STATUS_USB_SLAVE_PLUG_IN                          31:31
#define INTERRUPT_STATUS_USB_SLAVE_PLUG_IN_NOT_ACTIVE               0
#define INTERRUPT_STATUS_USB_SLAVE_PLUG_IN_ACTIVE                   1
#define INTERRUPT_STATUS_GPIO54                                     30:30
#define INTERRUPT_STATUS_GPIO54_NOT_ACTIVE                          0
#define INTERRUPT_STATUS_GPIO54_ACTIVE                              1
#define INTERRUPT_STATUS_GPIO53                                     29:29
#define INTERRUPT_STATUS_GPIO53_NOT_ACTIVE                          0
#define INTERRUPT_STATUS_GPIO53_ACTIVE                              1
#define INTERRUPT_STATUS_GPIO52                                     28:28
#define INTERRUPT_STATUS_GPIO52_NOT_ACTIVE                          0
#define INTERRUPT_STATUS_GPIO52_ACTIVE                              1
#define INTERRUPT_STATUS_GPIO51                                     27:27
#define INTERRUPT_STATUS_GPIO51_NOT_ACTIVE                          0
#define INTERRUPT_STATUS_GPIO51_ACTIVE                              1
#define INTERRUPT_STATUS_GPIO50                                     26:26
#define INTERRUPT_STATUS_GPIO50_NOT_ACTIVE                          0
#define INTERRUPT_STATUS_GPIO50_ACTIVE                              1
#define INTERRUPT_STATUS_GPIO49                                     25:25
#define INTERRUPT_STATUS_GPIO49_NOT_ACTIVE                          0
#define INTERRUPT_STATUS_GPIO49_ACTIVE                              1
#define INTERRUPT_STATUS_GPIO48                                     24:24
#define INTERRUPT_STATUS_GPIO48_NOT_ACTIVE                          0
#define INTERRUPT_STATUS_GPIO48_ACTIVE                              1
#define INTERRUPT_STATUS_I2C                                        23:23
#define INTERRUPT_STATUS_I2C_NOT_ACTIVE                             0
#define INTERRUPT_STATUS_I2C_ACTIVE                                 1
#define INTERRUPT_STATUS_PWM                                        22:22
#define INTERRUPT_STATUS_PWM_NOT_ACTIVE                             0
#define INTERRUPT_STATUS_PWM_ACTIVE                                 1
#define INTERRUPT_STATUS_DMA                                        20:20
#define INTERRUPT_STATUS_DMA_NOT_ACTIVE                             0
#define INTERRUPT_STATUS_DMA_ACTIVE                                 1
#define INTERRUPT_STATUS_PCI                                        19:19
#define INTERRUPT_STATUS_PCI_NOT_ACTIVE                             0
#define INTERRUPT_STATUS_PCI_ACTIVE                                 1
#define INTERRUPT_STATUS_I2S                                        18:18
#define INTERRUPT_STATUS_I2S_NOT_ACTIVE                             0
#define INTERRUPT_STATUS_I2S_ACTIVE                                 1
#define INTERRUPT_STATUS_AC97                                       17:17
#define INTERRUPT_STATUS_AC97_NOT_ACTIVE                            0
#define INTERRUPT_STATUS_AC97_ACTIVE                                1
#define INTERRUPT_STATUS_USB_SLAVE                                  16:16
#define INTERRUPT_STATUS_USB_SLAVE_NOT_ACTIVE                       0
#define INTERRUPT_STATUS_USB_SLAVE_ACTIVE                           1
#define INTERRUPT_STATUS_UART1                                      13:13
#define INTERRUPT_STATUS_UART1_NOT_ACTIVE                           0
#define INTERRUPT_STATUS_UART1_ACTIVE                               1
#define INTERRUPT_STATUS_UART0                                      12:12
#define INTERRUPT_STATUS_UART0_NOT_ACTIVE                           0
#define INTERRUPT_STATUS_UART0_ACTIVE                               1
#define INTERRUPT_STATUS_CRT_VERTICAL_SYNC                          11:11
#define INTERRUPT_STATUS_CRT_VERTICAL_SYNC_NOT_ACTIVE               0
#define INTERRUPT_STATUS_CRT_VERTICAL_SYNC_ACTIVE                   1
#define INTERRUPT_STATUS_8051                                       10:10
#define INTERRUPT_STATUS_8051_NOT_ACTIVE                            0
#define INTERRUPT_STATUS_8051_ACTIVE                                1
#define INTERRUPT_STATUS_SSP1                                       9:9
#define INTERRUPT_STATUS_SSP1_NOT_ACTIVE                            0
#define INTERRUPT_STATUS_SSP1_ACTIVE                                1
#define INTERRUPT_STATUS_SSP0                                       8:8
#define INTERRUPT_STATUS_SSP0_NOT_ACTIVE                            0
#define INTERRUPT_STATUS_SSP0_ACTIVE                                1
#define INTERRUPT_STATUS_USB_HOST                                   6:6
#define INTERRUPT_STATUS_USB_HOST_NOT_ACTIVE                        0
#define INTERRUPT_STATUS_USB_HOST_ACTIVE                            1
#define INTERRUPT_STATUS_2D                                         3:3
#define INTERRUPT_STATUS_2D_NOT_ACTIVE                              0
#define INTERRUPT_STATUS_2D_ACTIVE                                  1
#define INTERRUPT_STATUS_ZVPORT                                     2:2
#define INTERRUPT_STATUS_ZVPORT_NOT_ACTIVE                          0
#define INTERRUPT_STATUS_ZVPORT_ACTIVE                              1
#define INTERRUPT_STATUS_PANEL_VERTICAL_SYNC                        1:1
#define INTERRUPT_STATUS_PANEL_VERTICAL_SYNC_NOT_ACTIVE             0
#define INTERRUPT_STATUS_PANEL_VERTICAL_SYNC_ACTIVE                 1
#define INTERRUPT_STATUS_COMMAND_LIST                               0:0
#define INTERRUPT_STATUS_COMMAND_LIST_NOT_ACTIVE                    0
#define INTERRUPT_STATUS_COMMAND_LIST_ACTIVE                        1

/* Interrupt Mask */
#define INTERRUPT_MASK                                              0x000030
#define INTERRUPT_MASK_USB_SLAVE_PLUG_IN                            31:31
#define INTERRUPT_MASK_USB_SLAVE_PLUG_IN_DISABLE                    0
#define INTERRUPT_MASK_USB_SLAVE_PLUG_IN_ENABLE                     1
#define INTERRUPT_MASK_GPIO54                                       30:30
#define INTERRUPT_MASK_GPIO54_DISABLE                               0
#define INTERRUPT_MASK_GPIO54_ENABLE                                1
#define INTERRUPT_MASK_GPIO53                                       29:29
#define INTERRUPT_MASK_GPIO53_DISABLE                               0
#define INTERRUPT_MASK_GPIO53_ENABLE                                1
#define INTERRUPT_MASK_GPIO52                                       28:28
#define INTERRUPT_MASK_GPIO52_DISABLE                               0
#define INTERRUPT_MASK_GPIO52_ENABLE                                1
#define INTERRUPT_MASK_GPIO51                                       27:27
#define INTERRUPT_MASK_GPIO51_DISABLE                               0
#define INTERRUPT_MASK_GPIO51_ENABLE                                1
#define INTERRUPT_MASK_GPIO50                                       26:26
#define INTERRUPT_MASK_GPIO50_DISABLE                               0
#define INTERRUPT_MASK_GPIO50_ENABLE                                1
#define INTERRUPT_MASK_GPIO49                                       25:25
#define INTERRUPT_MASK_GPIO49_DISABLE                               0
#define INTERRUPT_MASK_GPIO49_ENABLE                                1
#define INTERRUPT_MASK_GPIO48                                       24:24
#define INTERRUPT_MASK_GPIO48_DISABLE                               0
#define INTERRUPT_MASK_GPIO48_ENABLE                                1
#define INTERRUPT_MASK_I2C                                          23:23
#define INTERRUPT_MASK_I2C_DISABLE                                  0
#define INTERRUPT_MASK_I2C_ENABLE                                   1
#define INTERRUPT_MASK_PWM                                          22:22
#define INTERRUPT_MASK_PWM_DISABLE                                  0
#define INTERRUPT_MASK_PWM_ENABLE                                   1
#define INTERRUPT_MASK_DMA                                          20:20
#define INTERRUPT_MASK_DMA_DISABLE                                  0
#define INTERRUPT_MASK_DMA_ENABLE                                   1
#define INTERRUPT_MASK_PCI                                          19:19
#define INTERRUPT_MASK_PCI_DISABLE                                  0
#define INTERRUPT_MASK_PCI_ENABLE                                   1
#define INTERRUPT_MASK_I2S                                          18:18
#define INTERRUPT_MASK_I2S_DISABLE                                  0
#define INTERRUPT_MASK_I2S_ENABLE                                   1
#define INTERRUPT_MASK_AC97                                         17:17
#define INTERRUPT_MASK_AC97_DISABLE                                 0
#define INTERRUPT_MASK_AC97_ENABLE                                  1
#define INTERRUPT_MASK_USB_SLAVE                                    16:16
#define INTERRUPT_MASK_USB_SLAVE_DISABLE                            0
#define INTERRUPT_MASK_USB_SLAVE_ENABLE                             1
#define INTERRUPT_MASK_UART1                                        13:13
#define INTERRUPT_MASK_UART1_DISABLE                                0
#define INTERRUPT_MASK_UART1_ENABLE                                 1
#define INTERRUPT_MASK_UART0                                        12:12
#define INTERRUPT_MASK_UART0_DISABLE                                0
#define INTERRUPT_MASK_UART0_ENABLE                                 1
#define INTERRUPT_MASK_CRT_VERTICAL_SYNC                            11:11
#define INTERRUPT_MASK_CRT_VERTICAL_SYNC_DISABLE                    0
#define INTERRUPT_MASK_CRT_VERTICAL_SYNC_ENABLE                     1
#define INTERRUPT_MASK_8051                                         10:10
#define INTERRUPT_MASK_8051_DISABLE                                 0
#define INTERRUPT_MASK_8051_ENABLE                                  1
#define INTERRUPT_MASK_SSP1                                         9:9
#define INTERRUPT_MASK_SSP1_DISABLE                                 0
#define INTERRUPT_MASK_SSP1_ENABLE                                  1
#define INTERRUPT_MASK_SSP0                                         8:8
#define INTERRUPT_MASK_SSP0_DISABLE                                 0
#define INTERRUPT_MASK_SSP0_ENABLE                                  1
#define INTERRUPT_MASK_USB_HOST                                     6:6
#define INTERRUPT_MASK_USB_HOST_DISABLE                             0
#define INTERRUPT_MASK_USB_HOST_ENABLE                              1
#define INTERRUPT_MASK_2D                                           3:3
#define INTERRUPT_MASK_2D_DISABLE                                   0
#define INTERRUPT_MASK_2D_ENABLE                                    1
#define INTERRUPT_MASK_ZVPORT                                       2:2
#define INTERRUPT_MASK_ZVPORT_DISABLE                               0
#define INTERRUPT_MASK_ZVPORT_ENABLE                                1
#define INTERRUPT_MASK_PANEL_VERTICAL_SYNC                          1:1
#define INTERRUPT_MASK_PANEL_VERTICAL_SYNC_DISABLE                  0
#define INTERRUPT_MASK_PANEL_VERTICAL_SYNC_ENABLE                   1
#define INTERRUPT_MASK_COMMAND_LIST                                 0:0
#define INTERRUPT_MASK_COMMAND_LIST_DISABLE                         0
#define INTERRUPT_MASK_COMMAND_LIST_ENABLE                          1

/* Debug Control */
#define DEBUG_CONTROL                                               0x000034
#define DEBUG_CONTROL_MODULE                                        7:5
#define DEBUG_CONTROL_PARTITION                                     4:0
#define DEBUG_CONTROL_PARTITION_HIF                                 0
#define DEBUG_CONTROL_PARTITION_EXTERNAL_MEMORY                     1
#define DEBUG_CONTROL_PARTITION_PCI                                 2
#define DEBUG_CONTROL_PARTITION_COMMAND_LIST                        3
#define DEBUG_CONTROL_PARTITION_DISPLAY                             4
#define DEBUG_CONTROL_PARTITION_ZVPORT                              5
#define DEBUG_CONTROL_PARTITION_2D                                  6
#define DEBUG_CONTROL_PARTITION_LOCAL_MEMORY                        8
#define DEBUG_CONTROL_PARTITION_USB_HOST                            10
#define DEBUG_CONTROL_PARTITION_SSP0                                12
#define DEBUG_CONTROL_PARTITION_SSP1                                13
#define DEBUG_CONTROL_PARTITION_UART0                               19
#define DEBUG_CONTROL_PARTITION_UART1                               20
#define DEBUG_CONTROL_PARTITION_I2C                                 21
#define DEBUG_CONTROL_PARTITION_8051                                23
#define DEBUG_CONTROL_PARTITION_AC97                                24
#define DEBUG_CONTROL_PARTITION_I2S                                 25
#define DEBUG_CONTROL_PARTITION_INTMEM                              26
#define DEBUG_CONTROL_PARTITION_DMA                                 27
#define DEBUG_CONTROL_PARTITION_SIMULATION                          28

/* Current Gate */
#define CURRENT_GATE                                                0x000038
#define CURRENT_GATE_AC97_I2S                                       18:18
#define CURRENT_GATE_AC97_I2S_DISABLE                               0
#define CURRENT_GATE_AC97_I2S_ENABLE                                1
#define CURRENT_GATE_8051                                           17:17
#define CURRENT_GATE_8051_DISABLE                                   0
#define CURRENT_GATE_8051_ENABLE                                    1
#define CURRENT_GATE_PLL                                            16:16
#define CURRENT_GATE_PLL_ENABLE                                     0
#define CURRENT_GATE_PLL_DISABLE                                    1
#define CURRENT_GATE_OSCILLATOR                                     15:15
#define CURRENT_GATE_OSCILLATOR_ENABLE                              0
#define CURRENT_GATE_OSCILLATOR_DISABLE                             1
#define CURRENT_GATE_PLL_RECOVERY                                   14:13
#define CURRENT_GATE_PLL_RECOVERY_1MS                               0
#define CURRENT_GATE_PLL_RECOVERY_2MS                               1
#define CURRENT_GATE_PLL_RECOVERY_3MS                               2
#define CURRENT_GATE_PLL_RECOVERY_4MS                               3
#define CURRENT_GATE_USB_SLAVE                                      12:12
#define CURRENT_GATE_USB_SLAVE_DISABLE                              0
#define CURRENT_GATE_USB_SLAVE_ENABLE                               1
#define CURRENT_GATE_USB_HOST                                       11:11
#define CURRENT_GATE_USB_HOST_DISABLE                               0
#define CURRENT_GATE_USB_HOST_ENABLE                                1
#define CURRENT_GATE_SSP                                            10:10
#define CURRENT_GATE_SSP_DISABLE                                    0
#define CURRENT_GATE_SSP_ENABLE                                     1
#define CURRENT_GATE_UART1                                          8:8
#define CURRENT_GATE_UART1_DISABLE                                  0
#define CURRENT_GATE_UART1_ENABLE                                   1
#define CURRENT_GATE_UART0                                          7:7
#define CURRENT_GATE_UART0_DISABLE                                  0
#define CURRENT_GATE_UART0_ENABLE                                   1
#define CURRENT_GATE_GPIO_PWM_I2C                                   6:6
#define CURRENT_GATE_GPIO_PWM_I2C_DISABLE                           0
#define CURRENT_GATE_GPIO_PWM_I2C_ENABLE                            1
#define CURRENT_GATE_ZVPORT                                         5:5
#define CURRENT_GATE_ZVPORT_DISABLE                                 0
#define CURRENT_GATE_ZVPORT_ENABLE                                  1
#define CURRENT_GATE_CSC                                            4:4
#define CURRENT_GATE_CSC_DISABLE                                    0
#define CURRENT_GATE_CSC_ENABLE                                     1
#define CURRENT_GATE_2D                                             3:3
#define CURRENT_GATE_2D_DISABLE                                     0
#define CURRENT_GATE_2D_ENABLE                                      1
#define CURRENT_GATE_DISPLAY                                        2:2
#define CURRENT_GATE_DISPLAY_DISABLE                                0
#define CURRENT_GATE_DISPLAY_ENABLE                                 1
#define CURRENT_GATE_MEMORY                                         1:1
#define CURRENT_GATE_MEMORY_DISABLE                                 0
#define CURRENT_GATE_MEMORY_ENABLE                                  1
#define CURRENT_GATE_HOST_COMMAND_LIST_DMA                          0:0
#define CURRENT_GATE_HOST_COMMAND_LIST_DMA_DISABLE                  0
#define CURRENT_GATE_HOST_COMMAND_LIST_DMA_ENABLE                   1

/* Current Clock */
#define CURRENT_CLOCK                                               0x00003C
#define CURRENT_CLOCK_P2XCLK_SELECT                                 29:29
#define CURRENT_CLOCK_P2XCLK_SELECT_228MHZ                          0
#define CURRENT_CLOCK_P2XCLK_SELECT_336MHZ                          1
#define CURRENT_CLOCK_P2XCLK_DIVIDER                                28:24
#define CURRENT_CLOCK_P2XCLK_DIVIDER_1                              0
#define CURRENT_CLOCK_P2XCLK_DIVIDER_2                              1
#define CURRENT_CLOCK_P2XCLK_DIVIDER_4                              2
#define CURRENT_CLOCK_P2XCLK_DIVIDER_8                              3
#define CURRENT_CLOCK_P2XCLK_DIVIDER_16                             4
#define CURRENT_CLOCK_P2XCLK_DIVIDER_32                             5
#define CURRENT_CLOCK_P2XCLK_DIVIDER_64                             6
#define CURRENT_CLOCK_P2XCLK_DIVIDER_128                            7
#define CURRENT_CLOCK_P2XCLK_DIVIDER_3                              8
#define CURRENT_CLOCK_P2XCLK_DIVIDER_6                              9
#define CURRENT_CLOCK_P2XCLK_DIVIDER_12                             10
#define CURRENT_CLOCK_P2XCLK_DIVIDER_24                             11
#define CURRENT_CLOCK_P2XCLK_DIVIDER_48                             12
#define CURRENT_CLOCK_P2XCLK_DIVIDER_96                             13
#define CURRENT_CLOCK_P2XCLK_DIVIDER_192                            14
#define CURRENT_CLOCK_P2XCLK_DIVIDER_384                            15
#define CURRENT_CLOCK_P2XCLK_DIVIDER_5                              16
#define CURRENT_CLOCK_P2XCLK_DIVIDER_10                             17
#define CURRENT_CLOCK_P2XCLK_DIVIDER_20                             18
#define CURRENT_CLOCK_P2XCLK_DIVIDER_40                             19
#define CURRENT_CLOCK_P2XCLK_DIVIDER_80                             20
#define CURRENT_CLOCK_P2XCLK_DIVIDER_160                            21
#define CURRENT_CLOCK_P2XCLK_DIVIDER_320                            22
#define CURRENT_CLOCK_P2XCLK_DIVIDER_640                            23
#define CURRENT_CLOCK_V2XCLK_SELECT                                 20:20
#define CURRENT_CLOCK_V2XCLK_SELECT_228MHZ                          0
#define CURRENT_CLOCK_V2XCLK_SELECT_336MHZ                          1
#define CURRENT_CLOCK_V2XCLK_DIVIDER                                19:16
#define CURRENT_CLOCK_V2XCLK_DIVIDER_1                              0
#define CURRENT_CLOCK_V2XCLK_DIVIDER_2                              1
#define CURRENT_CLOCK_V2XCLK_DIVIDER_4                              2
#define CURRENT_CLOCK_V2XCLK_DIVIDER_8                              3
#define CURRENT_CLOCK_V2XCLK_DIVIDER_16                             4
#define CURRENT_CLOCK_V2XCLK_DIVIDER_32                             5
#define CURRENT_CLOCK_V2XCLK_DIVIDER_64                             6
#define CURRENT_CLOCK_V2XCLK_DIVIDER_128                            7
#define CURRENT_CLOCK_V2XCLK_DIVIDER_3                              8
#define CURRENT_CLOCK_V2XCLK_DIVIDER_6                              9
#define CURRENT_CLOCK_V2XCLK_DIVIDER_12                             10
#define CURRENT_CLOCK_V2XCLK_DIVIDER_24                             11
#define CURRENT_CLOCK_V2XCLK_DIVIDER_48                             12
#define CURRENT_CLOCK_V2XCLK_DIVIDER_96                             13
#define CURRENT_CLOCK_V2XCLK_DIVIDER_192                            14
#define CURRENT_CLOCK_V2XCLK_DIVIDER_384                            15
#define CURRENT_CLOCK_MCLK_SELECT                                   12:12
#define CURRENT_CLOCK_MCLK_SELECT_228MHZ                            0
#define CURRENT_CLOCK_MCLK_SELECT_336MHZ                            1
#define CURRENT_CLOCK_MCLK_DIVIDER                                  11:8
#define CURRENT_CLOCK_MCLK_DIVIDER_1                                0
#define CURRENT_CLOCK_MCLK_DIVIDER_2                                1
#define CURRENT_CLOCK_MCLK_DIVIDER_4                                2
#define CURRENT_CLOCK_MCLK_DIVIDER_8                                3
#define CURRENT_CLOCK_MCLK_DIVIDER_16                               4
#define CURRENT_CLOCK_MCLK_DIVIDER_32                               5
#define CURRENT_CLOCK_MCLK_DIVIDER_64                               6
#define CURRENT_CLOCK_MCLK_DIVIDER_128                              7
#define CURRENT_CLOCK_MCLK_DIVIDER_3                                8
#define CURRENT_CLOCK_MCLK_DIVIDER_6                                9
#define CURRENT_CLOCK_MCLK_DIVIDER_12                               10
#define CURRENT_CLOCK_MCLK_DIVIDER_24                               11
#define CURRENT_CLOCK_MCLK_DIVIDER_48                               12
#define CURRENT_CLOCK_MCLK_DIVIDER_96                               13
#define CURRENT_CLOCK_MCLK_DIVIDER_192                              14
#define CURRENT_CLOCK_MCLK_DIVIDER_384                              15
#define CURRENT_CLOCK_M1XCLK_SELECT                                 4:4
#define CURRENT_CLOCK_M1XCLK_SELECT_228MHZ                          0
#define CURRENT_CLOCK_M1XCLK_SELECT_336MHZ                          1
#define CURRENT_CLOCK_M1XCLK_DIVIDER                                3:0
#define CURRENT_CLOCK_M1XCLK_DIVIDER_1                              0
#define CURRENT_CLOCK_M1XCLK_DIVIDER_2                              1
#define CURRENT_CLOCK_M1XCLK_DIVIDER_4                              2
#define CURRENT_CLOCK_M1XCLK_DIVIDER_8                              3
#define CURRENT_CLOCK_M1XCLK_DIVIDER_16                             4
#define CURRENT_CLOCK_M1XCLK_DIVIDER_32                             5
#define CURRENT_CLOCK_M1XCLK_DIVIDER_64                             6
#define CURRENT_CLOCK_M1XCLK_DIVIDER_128                            7
#define CURRENT_CLOCK_M1XCLK_DIVIDER_3                              8
#define CURRENT_CLOCK_M1XCLK_DIVIDER_6                              9
#define CURRENT_CLOCK_M1XCLK_DIVIDER_12                             10
#define CURRENT_CLOCK_M1XCLK_DIVIDER_24                             11
#define CURRENT_CLOCK_M1XCLK_DIVIDER_48                             12
#define CURRENT_CLOCK_M1XCLK_DIVIDER_96                             13
#define CURRENT_CLOCK_M1XCLK_DIVIDER_192                            14
#define CURRENT_CLOCK_M1XCLK_DIVIDER_384                            15

/* Power Mode 0 Gate */
#define POWER_MODE0_GATE                                            0x000040
#define POWER_MODE0_GATE_AC97_I2S                                   18:18
#define POWER_MODE0_GATE_AC97_I2S_DISABLE                           0
#define POWER_MODE0_GATE_AC97_I2S_ENABLE                            1
#define POWER_MODE0_GATE_8051                                       17:17
#define POWER_MODE0_GATE_8051_DISABLE                               0
#define POWER_MODE0_GATE_8051_ENABLE                                1
#define POWER_MODE0_GATE_USB_SLAVE                                  12:12
#define POWER_MODE0_GATE_USB_SLAVE_DISABLE                          0
#define POWER_MODE0_GATE_USB_SLAVE_ENABLE                           1
#define POWER_MODE0_GATE_USB_HOST                                   11:11
#define POWER_MODE0_GATE_USB_HOST_DISABLE                           0
#define POWER_MODE0_GATE_USB_HOST_ENABLE                            1
#define POWER_MODE0_GATE_SSP                                        10:10
#define POWER_MODE0_GATE_SSP_DISABLE                                0
#define POWER_MODE0_GATE_SSP_ENABLE                                 1
#define POWER_MODE0_GATE_UART1                                      8:8
#define POWER_MODE0_GATE_UART1_DISABLE                              0
#define POWER_MODE0_GATE_UART1_ENABLE                               1
#define POWER_MODE0_GATE_UART0                                      7:7
#define POWER_MODE0_GATE_UART0_DISABLE                              0
#define POWER_MODE0_GATE_UART0_ENABLE                               1
#define POWER_MODE0_GATE_GPIO_PWM_I2C                               6:6
#define POWER_MODE0_GATE_GPIO_PWM_I2C_DISABLE                       0
#define POWER_MODE0_GATE_GPIO_PWM_I2C_ENABLE                        1
#define POWER_MODE0_GATE_ZVPORT                                     5:5
#define POWER_MODE0_GATE_ZVPORT_DISABLE                             0
#define POWER_MODE0_GATE_ZVPORT_ENABLE                              1
#define POWER_MODE0_GATE_CSC                                        4:4
#define POWER_MODE0_GATE_CSC_DISABLE                                0
#define POWER_MODE0_GATE_CSC_ENABLE                                 1
#define POWER_MODE0_GATE_2D                                         3:3
#define POWER_MODE0_GATE_2D_DISABLE                                 0
#define POWER_MODE0_GATE_2D_ENABLE                                  1
#define POWER_MODE0_GATE_DISPLAY                                    2:2
#define POWER_MODE0_GATE_DISPLAY_DISABLE                            0
#define POWER_MODE0_GATE_DISPLAY_ENABLE                             1
#define POWER_MODE0_GATE_MEMORY                                     1:1
#define POWER_MODE0_GATE_MEMORY_DISABLE                             0
#define POWER_MODE0_GATE_MEMORY_ENABLE                              1
#define POWER_MODE0_GATE_HOST_COMMAND_LIST_DMA                      0:0
#define POWER_MODE0_GATE_HOST_COMMAND_LIST_DMA_DISABLE              0
#define POWER_MODE0_GATE_HOST_COMMAND_LIST_DMA_ENABLE               1

/* Power Mode 0 Clock */
#define POWER_MODE0_CLOCK                                           0x000044
#define POWER_MODE0_CLOCK_P2XCLK_SELECT                             29:29
#define POWER_MODE0_CLOCK_P2XCLK_SELECT_228MHZ                      0
#define POWER_MODE0_CLOCK_P2XCLK_SELECT_336MHZ                      1
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER                            28:24
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_1                          0
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_2                          1
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_4                          2
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_8                          3
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_16                         4
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_32                         5
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_64                         6
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_128                        7
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_3                          8
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_6                          9
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_12                         10
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_24                         11
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_48                         12
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_96                         13
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_192                        14
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_384                        15
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_5                          16
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_10                         17
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_20                         18
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_40                         19
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_80                         20
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_160                        21
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_320                        22
#define POWER_MODE0_CLOCK_P2XCLK_DIVIDER_640                        23
#define POWER_MODE0_CLOCK_V2XCLK_SELECT                             20:20
#define POWER_MODE0_CLOCK_V2XCLK_SELECT_228MHZ                      0
#define POWER_MODE0_CLOCK_V2XCLK_SELECT_336MHZ                      1
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER                            19:16
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_1                          0
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_2                          1
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_4                          2
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_8                          3
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_16                         4
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_32                         5
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_64                         6
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_128                        7
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_3                          8
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_6                          9
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_12                         10
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_24                         11
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_48                         12
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_96                         13
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_192                        14
#define POWER_MODE0_CLOCK_V2XCLK_DIVIDER_384                        15
#define POWER_MODE0_CLOCK_MCLK_SELECT                               12:12
#define POWER_MODE0_CLOCK_MCLK_SELECT_228MHZ                        0
#define POWER_MODE0_CLOCK_MCLK_SELECT_336MHZ                        1
#define POWER_MODE0_CLOCK_MCLK_DIVIDER                              11:8
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_1                            0
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_2                            1
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_4                            2
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_8                            3
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_16                           4
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_32                           5
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_64                           6
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_128                          7
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_3                            8
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_6                            9
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_12                           10
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_24                           11
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_48                           12
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_96                           13
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_192                          14
#define POWER_MODE0_CLOCK_MCLK_DIVIDER_384                          15
#define POWER_MODE0_CLOCK_M1XCLK_SELECT                             4:4
#define POWER_MODE0_CLOCK_M1XCLK_SELECT_228MHZ                      0
#define POWER_MODE0_CLOCK_M1XCLK_SELECT_336MHZ                      1
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER                            3:0
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_1                          0
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_2                          1
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_4                          2
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_8                          3
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_16                         4
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_32                         5
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_64                         6
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_128                        7
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_3                          8
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_6                          9
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_12                         10
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_24                         11
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_48                         12
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_96                         13
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_192                        14
#define POWER_MODE0_CLOCK_M1XCLK_DIVIDER_384                        15

/* Power Mode 1 Gate */
#define POWER_MODE1_GATE                                            0x000048
#define POWER_MODE1_GATE_AC97_I2S                                   18:18
#define POWER_MODE1_GATE_AC97_I2S_DISABLE                           0
#define POWER_MODE1_GATE_AC97_I2S_ENABLE                            1
#define POWER_MODE1_GATE_8051                                       17:17
#define POWER_MODE1_GATE_8051_DISABLE                               0
#define POWER_MODE1_GATE_8051_ENABLE                                1
#define POWER_MODE1_GATE_USB_SLAVE                                  12:12
#define POWER_MODE1_GATE_USB_SLAVE_DISABLE                          0
#define POWER_MODE1_GATE_USB_SLAVE_ENABLE                           1
#define POWER_MODE1_GATE_USB_HOST                                   11:11
#define POWER_MODE1_GATE_USB_HOST_DISABLE                           0
#define POWER_MODE1_GATE_USB_HOST_ENABLE                            1
#define POWER_MODE1_GATE_SSP                                        10:10
#define POWER_MODE1_GATE_SSP_DISABLE                                0
#define POWER_MODE1_GATE_SSP_ENABLE                                 1
#define POWER_MODE1_GATE_UART1                                      8:8
#define POWER_MODE1_GATE_UART1_DISABLE                              0
#define POWER_MODE1_GATE_UART1_ENABLE                               1
#define POWER_MODE1_GATE_UART0                                      7:7
#define POWER_MODE1_GATE_UART0_DISABLE                              0
#define POWER_MODE1_GATE_UART0_ENABLE                               1
#define POWER_MODE1_GATE_GPIO_PWM_I2C                               6:6
#define POWER_MODE1_GATE_GPIO_PWM_I2C_DISABLE                       0
#define POWER_MODE1_GATE_GPIO_PWM_I2C_ENABLE                        1
#define POWER_MODE1_GATE_ZVPORT                                     5:5
#define POWER_MODE1_GATE_ZVPORT_DISABLE                             0
#define POWER_MODE1_GATE_ZVPORT_ENABLE                              1
#define POWER_MODE1_GATE_CSC                                        4:4
#define POWER_MODE1_GATE_CSC_DISABLE                                0
#define POWER_MODE1_GATE_CSC_ENABLE                                 1
#define POWER_MODE1_GATE_2D                                         3:3
#define POWER_MODE1_GATE_2D_DISABLE                                 0
#define POWER_MODE1_GATE_2D_ENABLE                                  1
#define POWER_MODE1_GATE_DISPLAY                                    2:2
#define POWER_MODE1_GATE_DISPLAY_DISABLE                            0
#define POWER_MODE1_GATE_DISPLAY_ENABLE                             1
#define POWER_MODE1_GATE_MEMORY                                     1:1
#define POWER_MODE1_GATE_MEMORY_DISABLE                             0
#define POWER_MODE1_GATE_MEMORY_ENABLE                              1
#define POWER_MODE1_GATE_HOST_COMMAND_LIST_DMA                      0:0
#define POWER_MODE1_GATE_HOST_COMMAND_LIST_DMA_DISABLE              0
#define POWER_MODE1_GATE_HOST_COMMAND_LIST_DMA_ENABLE               1

/* Power Mode 1 Clock */
#define POWER_MODE1_CLOCK                                           0x00004C
#define POWER_MODE1_CLOCK_P2XCLK_SELECT                             29:29
#define POWER_MODE1_CLOCK_P2XCLK_SELECT_228MHZ                      0
#define POWER_MODE1_CLOCK_P2XCLK_SELECT_336MHZ                      1
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER                            28:24
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_1                          0
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_2                          1
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_4                          2
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_8                          3
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_16                         4
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_32                         5
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_64                         6
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_128                        7
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_3                          8
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_6                          9
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_12                         10
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_24                         11
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_48                         12
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_96                         13
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_192                        14
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_384                        15
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_5                          16
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_10                         17
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_20                         18
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_40                         19
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_80                         20
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_160                        21
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_320                        22
#define POWER_MODE1_CLOCK_P2XCLK_DIVIDER_640                        23
#define POWER_MODE1_CLOCK_V2XCLK_SELECT                             20:20
#define POWER_MODE1_CLOCK_V2XCLK_SELECT_228MHZ                      0
#define POWER_MODE1_CLOCK_V2XCLK_SELECT_336MHZ                      1
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER                            19:16
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_1                          0
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_2                          1
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_4                          2
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_8                          3
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_16                         4
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_32                         5
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_64                         6
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_128                        7
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_3                          8
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_6                          9
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_12                         10
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_24                         11
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_48                         12
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_96                         13
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_192                        14
#define POWER_MODE1_CLOCK_V2XCLK_DIVIDER_384                        15
#define POWER_MODE1_CLOCK_MCLK_SELECT                               12:12
#define POWER_MODE1_CLOCK_MCLK_SELECT_228MHZ                        0
#define POWER_MODE1_CLOCK_MCLK_SELECT_336MHZ                        1
#define POWER_MODE1_CLOCK_MCLK_DIVIDER                              11:8
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_1                            0
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_2                            1
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_4                            2
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_8                            3
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_16                           4
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_32                           5
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_64                           6
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_128                          7
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_3                            8
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_6                            9
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_12                           10
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_24                           11
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_48                           12
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_96                           13
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_192                          14
#define POWER_MODE1_CLOCK_MCLK_DIVIDER_384                          15
#define POWER_MODE1_CLOCK_M1XCLK_SELECT                             4:4
#define POWER_MODE1_CLOCK_M1XCLK_SELECT_228MHZ                      0
#define POWER_MODE1_CLOCK_M1XCLK_SELECT_336MHZ                      1
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER                            3:0
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_1                          0
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_2                          1
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_4                          2
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_8                          3
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_16                         4
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_32                         5
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_64                         6
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_128                        7
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_3                          8
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_6                          9
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_12                         10
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_24                         11
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_48                         12
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_96                         13
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_192                        14
#define POWER_MODE1_CLOCK_M1XCLK_DIVIDER_384                        15

/* Sleep Mode Gate */
#define SLEEP_MODE_GATE                                             0x000050
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK                          22:19
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK_4096                     0
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK_2048                     1
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK_1024                     2
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK_512                      3
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK_256                      4
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK_128                      5
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK_64                       6
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK_32                       7
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK_16                       8
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK_8                        9
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK_4                        10
#define SLEEP_MODE_GATE_PLL_RECOVERY_CLOCK_2                        11
#define SLEEP_MODE_GATE_PLL_RECOVERY                                14:13
#define SLEEP_MODE_GATE_PLL_RECOVERY_1MS                            0
#define SLEEP_MODE_GATE_PLL_RECOVERY_2MS                            1
#define SLEEP_MODE_GATE_PLL_RECOVERY_3MS                            2
#define SLEEP_MODE_GATE_PLL_RECOVERY_4MS                            3

/* Power Mode Control */
#define POWER_MODE_CONTROL                                          0x000054
#define POWER_MODE_CONTROL_STATUS                                   2:2
#define POWER_MODE_CONTROL_STATUS_NOT_SLEEP                         0
#define POWER_MODE_CONTROL_STATUS_SLEEP                             1
#define POWER_MODE_CONTROL_MODE_SELECT                              1:0
#define POWER_MODE_CONTROL_MODE_SELECT_MODE0                        0
#define POWER_MODE_CONTROL_MODE_SELECT_MODE1                        1
#define POWER_MODE_CONTROL_MODE_SELECT_SLEEP                        2

/* PCI Master Base Address */
#define PCI_MASTER_BASE                                             0x000058
#define PCI_MASTER_BASE_ADDRESS                                     31:20

/* Endian Control */
#define ENDIAN_CONTROL                                              0x00005C
#define ENDIAN_CONTROL_ENDIAN_SELECT                                0:0
#define ENDIAN_CONTROL_ENDIAN_SELECT_LITTLE                         0
#define ENDIAN_CONTROL_ENDIAN_SELECT_BIG                            1

/* Device Id */
#define DEVICE_ID                                                   0x000060
#define DEVICE_ID_DEVICE                                            31:16
#define DEVICE_ID_REVISION                                          7:0

/* PLL Clock Count */
#define PLL_CLOCK_COUNT                                             0x000064
#define PLL_CLOCK_COUNT_COUNT                                       15:0

/* Miscellaneous Timing */
#define MISCELLANEOUS_TIMING                                        0x000068
#define MISCELLANEOUS_TIMING_EXTEND                                 31:28
#define MISCELLANEOUS_TIMING_EXTEND_NONE                            0
#define MISCELLANEOUS_TIMING_EXTEND_16_HCLK                         1
#define MISCELLANEOUS_TIMING_EXTEND_32_HCLK                         2
#define MISCELLANEOUS_TIMING_EXTEND_48_HCLK                         3
#define MISCELLANEOUS_TIMING_EXTEND_64_HCLK                         4
#define MISCELLANEOUS_TIMING_EXTEND_80_HCLK                         5
#define MISCELLANEOUS_TIMING_EXTEND_96_HCLK                         6
#define MISCELLANEOUS_TIMING_EXTEND_112_HCLK                        7
#define MISCELLANEOUS_TIMING_EXTEND_128_HCLK                        8
#define MISCELLANEOUS_TIMING_EXTEND_144_HCLK                        9
#define MISCELLANEOUS_TIMING_EXTEND_160_HCLK                        10
#define MISCELLANEOUS_TIMING_EXTEND_176_HCLK                        11
#define MISCELLANEOUS_TIMING_EXTEND_192_HCLK                        12
#define MISCELLANEOUS_TIMING_EXTEND_208_HCLK                        13
#define MISCELLANEOUS_TIMING_EXTEND_224_HCLK                        14
#define MISCELLANEOUS_TIMING_EXTEND_240_HCLK                        15
#define MISCELLANEOUS_TIMING_XSCALE                                 25:24
#define MISCELLANEOUS_TIMING_XSCALE_PLL                             0
#define MISCELLANEOUS_TIMING_XSCALE_HCLK                            1
#define MISCELLANEOUS_TIMING_XSCALE_GPIO30                          2
#define MISCELLANEOUS_TIMING_USB_HOST_OVER_CURRENT                  23:23
#define MISCELLANEOUS_TIMING_USB_HOST_OVER_CURRENT_DISABLE          0
#define MISCELLANEOUS_TIMING_USB_HOST_OVER_CURRENT_ENABLE           1
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_SELECT_MODE1              20:20
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_SELECT_MODE1_288          0
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_SELECT_MODE1_336          1
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1             19:16
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_1           0
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_2           1
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_4           2
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_8           3
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_16          4
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_32          5
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_64          6
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_128         7
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_3           8
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_6           9
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_12          10
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_24          11
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_48          12
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_96          13
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_192         14
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE1_384         15
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_SELECT_MODE0              12:12
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_SELECT_MODE0_288          0
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_SELECT_MODE0_336          1
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0             11:8
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_1           0
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_2           1
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_4           2
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_8           3
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_16          4
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_32          5
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_64          6
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_128         7
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_3           8
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_6           9
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_12          10
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_24          11
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_48          12
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_96          13
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_192         14
#define MISCELLANEOUS_TIMING_SYSTEM_SDRAM_DIVIDER_MODE0_384         15
#define MISCELLANEOUS_TIMING_96MHZ_PLL_DEBUG                        7:7
#define MISCELLANEOUS_TIMING_96MHZ_PLL_DEBUG_CHECK_INPUT            0
#define MISCELLANEOUS_TIMING_96MHZ_PLL_DEBUG_CHECK_OUTPUT           1
#define MISCELLANEOUS_TIMING_ACPI_CONTROL                           6:6
#define MISCELLANEOUS_TIMING_ACPI_CONTROL_DISABLE                   0
#define MISCELLANEOUS_TIMING_ACPI_CONTROL_ENABLE                    1
#define MISCELLANEOUS_TIMING_PLL_DIVIDER                            5:4
#define MISCELLANEOUS_TIMING_PLL_DIVIDER_336                        0
#define MISCELLANEOUS_TIMING_PLL_DIVIDER_288                        1
#define MISCELLANEOUS_TIMING_PLL_DIVIDER_240                        2
#define MISCELLANEOUS_TIMING_PLL_DIVIDER_192                        3
#define MISCELLANEOUS_TIMING_USB_HOST_MODE                          3:3
#define MISCELLANEOUS_TIMING_USB_HOST_MODE_NORMAL                   0
#define MISCELLANEOUS_TIMING_USB_HOST_MODE_SIMULATION               1
#define MISCELLANEOUS_TIMING_DELAY                                  2:0
#define MISCELLANEOUS_TIMING_DELAY_NONE                             0
#define MISCELLANEOUS_TIMING_DELAY_500PS                            1
#define MISCELLANEOUS_TIMING_DELAY_1NS                              2
#define MISCELLANEOUS_TIMING_DELAY_1500PS                           3
#define MISCELLANEOUS_TIMING_DELAY_2NS                              4
#define MISCELLANEOUS_TIMING_DELAY_2500PS                           5

/* Current System SDRAM Clock */
#define CURRENT_SYSTEM_SDRAM_CLOCK                                  0x00006C
#define CURRENT_SYSTEM_SDRAM_CLOCK_SELECT                           4:4
#define CURRENT_SYSTEM_SDRAM_CLOCK_SELECT_288                       0
#define CURRENT_SYSTEM_SDRAM_CLOCK_SELECT_336                       1
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER                          3:0
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_1                        0
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_2                        1
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_4                        2
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_8                        3
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_16                       4
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_32                       5
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_64                       6
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_128                      7
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_3                        8
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_6                        9
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_12                       10
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_24                       11
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_48                       12
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_96                       13
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_192                      14
#define CURRENT_SYSTEM_SDRAM_CLOCK_DIVIDER_384                      15

/*****************************************************************************\
 *                              GPIO / PWM / I2C                             *
\*****************************************************************************/

/* GPIO[31:0] Data */
#define GPIO_DATA_LOW                                               0x010000

/* GPIO[63:32] Data */
#define GPIO_DATA_HIGH                                              0x010004

/* GPIO[31:0] Data Direction */
#define GPIO_DATA_DIRECTION_LOW                                     0x010008
#define GPIO_DATA_DIRECTION_LOW_INPUT                               0
#define GPIO_DATA_DIRECTION_LOW_OUTPUT                              1

/* GPIO[63:32] Data Direction */
#define GPIO_DATA_DIRECTION_HIGH                                    0x01000C
#define GPIO_DATA_DIRECTION_HIGH_INPUT                              0
#define GPIO_DATA_DIRECTION_HIGH_OUTPUT                             1

/* GPIO Interrupt Setup */
#define GPIO_INTERRUPT_SETUP                                        0x010010
#define GPIO_INTERRUPT_SETUP_TRIGGER_54                             22:22
#define GPIO_INTERRUPT_SETUP_TRIGGER_54_EDGE                        0
#define GPIO_INTERRUPT_SETUP_TRIGGER_54_LEVEL                       1
#define GPIO_INTERRUPT_SETUP_TRIGGER_53                             21:21
#define GPIO_INTERRUPT_SETUP_TRIGGER_53_EDGE                        0
#define GPIO_INTERRUPT_SETUP_TRIGGER_53_LEVEL                       1
#define GPIO_INTERRUPT_SETUP_TRIGGER_52                             20:20
#define GPIO_INTERRUPT_SETUP_TRIGGER_52_EDGE                        0
#define GPIO_INTERRUPT_SETUP_TRIGGER_52_LEVEL                       1
#define GPIO_INTERRUPT_SETUP_TRIGGER_51                             19:19
#define GPIO_INTERRUPT_SETUP_TRIGGER_51_EDGE                        0
#define GPIO_INTERRUPT_SETUP_TRIGGER_51_LEVEL                       1
#define GPIO_INTERRUPT_SETUP_TRIGGER_50                             18:18
#define GPIO_INTERRUPT_SETUP_TRIGGER_50_EDGE                        0
#define GPIO_INTERRUPT_SETUP_TRIGGER_50_LEVEL                       1
#define GPIO_INTERRUPT_SETUP_TRIGGER_49                             17:17
#define GPIO_INTERRUPT_SETUP_TRIGGER_49_EDGE                        0
#define GPIO_INTERRUPT_SETUP_TRIGGER_49_LEVEL                       1
#define GPIO_INTERRUPT_SETUP_TRIGGER_48                             16:16
#define GPIO_INTERRUPT_SETUP_TRIGGER_48_EDGE                        0
#define GPIO_INTERRUPT_SETUP_TRIGGER_48_LEVEL                       1
#define GPIO_INTERRUPT_SETUP_ACTIVE_54                              14:14
#define GPIO_INTERRUPT_SETUP_ACTIVE_54_LOW_FALLING_EDGE             0
#define GPIO_INTERRUPT_SETUP_ACTIVE_54_HIGH_RISING_EDGE             1
#define GPIO_INTERRUPT_SETUP_ACTIVE_53                              13:13
#define GPIO_INTERRUPT_SETUP_ACTIVE_53_LOW_FALLING_EDGE             0
#define GPIO_INTERRUPT_SETUP_ACTIVE_53_HIGH_RISING_EDGE             1
#define GPIO_INTERRUPT_SETUP_ACTIVE_52                              12:12
#define GPIO_INTERRUPT_SETUP_ACTIVE_52_LOW_FALLING_EDGE             0
#define GPIO_INTERRUPT_SETUP_ACTIVE_52_HIGH_RISING_EDGE             1
#define GPIO_INTERRUPT_SETUP_ACTIVE_51                              11:11
#define GPIO_INTERRUPT_SETUP_ACTIVE_51_LOW_FALLING_EDGE             0
#define GPIO_INTERRUPT_SETUP_ACTIVE_51_HIGH_RISING_EDGE             1
#define GPIO_INTERRUPT_SETUP_ACTIVE_50                              10:10
#define GPIO_INTERRUPT_SETUP_ACTIVE_50_LOW_FALLING_EDGE             0
#define GPIO_INTERRUPT_SETUP_ACTIVE_50_HIGH_RISING_EDGE             1
#define GPIO_INTERRUPT_SETUP_ACTIVE_49                              9:9
#define GPIO_INTERRUPT_SETUP_ACTIVE_49_LOW_FALLING_EDGE             0
#define GPIO_INTERRUPT_SETUP_ACTIVE_49_HIGH_RISING_EDGE             1
#define GPIO_INTERRUPT_SETUP_ACTIVE_48                              8:8
#define GPIO_INTERRUPT_SETUP_ACTIVE_48_LOW_FALLING_EDGE             0
#define GPIO_INTERRUPT_SETUP_ACTIVE_48_HIGH_RISING_EDGE             1
#define GPIO_INTERRUPT_SETUP_ENABLE_54                              6:6
#define GPIO_INTERRUPT_SETUP_ENABLE_54_INPUT_OUTPUT                 0
#define GPIO_INTERRUPT_SETUP_ENABLE_54_INTERRUPT                    1
#define GPIO_INTERRUPT_SETUP_ENABLE_53                              5:5
#define GPIO_INTERRUPT_SETUP_ENABLE_53_INPUT_OUTPUT                 0
#define GPIO_INTERRUPT_SETUP_ENABLE_53_INTERRUPT                    1
#define GPIO_INTERRUPT_SETUP_ENABLE_52                              4:4
#define GPIO_INTERRUPT_SETUP_ENABLE_52_INPUT_OUTPUT                 0
#define GPIO_INTERRUPT_SETUP_ENABLE_52_INTERRUPT                    1
#define GPIO_INTERRUPT_SETUP_ENABLE_51                              3:3
#define GPIO_INTERRUPT_SETUP_ENABLE_51_INPUT_OUTPUT                 0
#define GPIO_INTERRUPT_SETUP_ENABLE_51_INTERRUPT                    1
#define GPIO_INTERRUPT_SETUP_ENABLE_50                              2:2
#define GPIO_INTERRUPT_SETUP_ENABLE_50_INPUT_OUTPUT                 0
#define GPIO_INTERRUPT_SETUP_ENABLE_50_INTERRUPT                    1
#define GPIO_INTERRUPT_SETUP_ENABLE_49                              1:1
#define GPIO_INTERRUPT_SETUP_ENABLE_49_INPUT_OUTPUT                 0
#define GPIO_INTERRUPT_SETUP_ENABLE_49_INTERRUPT                    1
#define GPIO_INTERRUPT_SETUP_ENABLE_48                              0:0
#define GPIO_INTERRUPT_SETUP_ENABLE_48_INPUT_OUTPUT                 0
#define GPIO_INTERRUPT_SETUP_ENABLE_48_INTERRUPT                    1

/* GPIO Interrupt Status & Reset */
#define GPIO_INTERRUPT_STATUS                                       0x010014
#define GPIO_INTERRUPT_STATUS_54                                    22:22
#define GPIO_INTERRUPT_STATUS_54_NOT_ACTIVE                         0
#define GPIO_INTERRUPT_STATUS_54_ACTIVE                             1
#define GPIO_INTERRUPT_STATUS_54_RESET                              1
#define GPIO_INTERRUPT_STATUS_53                                    21:21
#define GPIO_INTERRUPT_STATUS_53_NOT_ACTIVE                         0
#define GPIO_INTERRUPT_STATUS_53_ACTIVE                             1
#define GPIO_INTERRUPT_STATUS_53_RESET                              1
#define GPIO_INTERRUPT_STATUS_52                                    20:20
#define GPIO_INTERRUPT_STATUS_52_NOT_ACTIVE                         0
#define GPIO_INTERRUPT_STATUS_52_ACTIVE                             1
#define GPIO_INTERRUPT_STATUS_52_RESET                              1
#define GPIO_INTERRUPT_STATUS_51                                    19:19
#define GPIO_INTERRUPT_STATUS_51_NOT_ACTIVE                         0
#define GPIO_INTERRUPT_STATUS_51_ACTIVE                             1
#define GPIO_INTERRUPT_STATUS_51_RESET                              1
#define GPIO_INTERRUPT_STATUS_50                                    18:18
#define GPIO_INTERRUPT_STATUS_50_NOT_ACTIVE                         0
#define GPIO_INTERRUPT_STATUS_50_ACTIVE                             1
#define GPIO_INTERRUPT_STATUS_50_RESET                              1
#define GPIO_INTERRUPT_STATUS_49                                    17:17
#define GPIO_INTERRUPT_STATUS_49_NOT_ACTIVE                         0
#define GPIO_INTERRUPT_STATUS_49_ACTIVE                             1
#define GPIO_INTERRUPT_STATUS_49_RESET                              1
#define GPIO_INTERRUPT_STATUS_48                                    16:16
#define GPIO_INTERRUPT_STATUS_48_NOT_ACTIVE                         0
#define GPIO_INTERRUPT_STATUS_48_ACTIVE                             1
#define GPIO_INTERRUPT_STATUS_48_RESET                              1

/* PWM 0 */
#define PWM0                                                        0x010020
#define PWM0_HIGH_COUNTER                                           31:20
#define PWM0_LOW_COUNTER                                            19:8
#define PWM0_CLOCK_DIVIDE                                           7:4
#define PWM0_CLOCK_DIVIDE_1                                         0
#define PWM0_CLOCK_DIVIDE_2                                         1
#define PWM0_CLOCK_DIVIDE_4                                         2
#define PWM0_CLOCK_DIVIDE_8                                         3
#define PWM0_CLOCK_DIVIDE_16                                        4
#define PWM0_CLOCK_DIVIDE_32                                        5
#define PWM0_CLOCK_DIVIDE_64                                        6
#define PWM0_CLOCK_DIVIDE_128                                       7
#define PWM0_CLOCK_DIVIDE_256                                       8
#define PWM0_CLOCK_DIVIDE_512                                       9
#define PWM0_CLOCK_DIVIDE_1024                                      10
#define PWM0_CLOCK_DIVIDE_2048                                      11
#define PWM0_CLOCK_DIVIDE_4096                                      12
#define PWM0_CLOCK_DIVIDE_8192                                      13
#define PWM0_CLOCK_DIVIDE_16384                                     14
#define PWM0_CLOCK_DIVIDE_32768                                     15
#define PWM0_INTERRUPT_STATUS                                       3:3
#define PWM0_INTERRUPT_STATUS_NOT_ACTIVE                            0
#define PWM0_INTERRUPT_STATUS_ACTIVE                                1
#define PWM0_INTERRUPT_CONTROL                                      2:2
#define PWM0_INTERRUPT_CONTROL_DISABLE                              0
#define PWM0_INTERRUPT_CONTROL_ENABLE                               1
#define PWM0_CONTROL                                                0:0
#define PWM0_CONTROL_DISABLE                                        0
#define PWM0_CONTROL_ENABLE                                         1

/* PWM 1 */
#define PWM1                                                        0x010024
#define PWM1_HIGH_COUNTER                                           31:20
#define PWM1_LOW_COUNTER                                            19:8
#define PWM1_CLOCK_DIVIDE                                           7:4
#define PWM1_CLOCK_DIVIDE_1                                         0
#define PWM1_CLOCK_DIVIDE_2                                         1
#define PWM1_CLOCK_DIVIDE_4                                         2
#define PWM1_CLOCK_DIVIDE_8                                         3
#define PWM1_CLOCK_DIVIDE_16                                        4
#define PWM1_CLOCK_DIVIDE_32                                        5
#define PWM1_CLOCK_DIVIDE_64                                        6
#define PWM1_CLOCK_DIVIDE_128                                       7
#define PWM1_CLOCK_DIVIDE_256                                       8
#define PWM1_CLOCK_DIVIDE_512                                       9
#define PWM1_CLOCK_DIVIDE_1024                                      10
#define PWM1_CLOCK_DIVIDE_2048                                      11
#define PWM1_CLOCK_DIVIDE_4096                                      12
#define PWM1_CLOCK_DIVIDE_8192                                      13
#define PWM1_CLOCK_DIVIDE_16384                                     14
#define PWM1_CLOCK_DIVIDE_32768                                     15
#define PWM1_INTERRUPT_STATUS                                       3:3
#define PWM1_INTERRUPT_STATUS_NOT_ACTIVE                            0
#define PWM1_INTERRUPT_STATUS_ACTIVE                                1
#define PWM1_INTERRUPT_CONTROL                                      2:2
#define PWM1_INTERRUPT_CONTROL_DISABLE                              0
#define PWM1_INTERRUPT_CONTROL_ENABLE                               1
#define PWM1_CONTROL                                                0:0
#define PWM1_CONTROL_DISABLE                                        0
#define PWM1_CONTROL_ENABLE                                         1

/* PWM 2 */
#define PWM2                                                        0x010028
#define PWM2_HIGH_COUNTER                                           31:20
#define PWM2_LOW_COUNTER                                            19:8
#define PWM2_CLOCK_DIVIDE                                           7:4
#define PWM2_CLOCK_DIVIDE_1                                         0
#define PWM2_CLOCK_DIVIDE_2                                         1
#define PWM2_CLOCK_DIVIDE_4                                         2
#define PWM2_CLOCK_DIVIDE_8                                         3
#define PWM2_CLOCK_DIVIDE_16                                        4
#define PWM2_CLOCK_DIVIDE_32                                        5
#define PWM2_CLOCK_DIVIDE_64                                        6
#define PWM2_CLOCK_DIVIDE_128                                       7
#define PWM2_CLOCK_DIVIDE_256                                       8
#define PWM2_CLOCK_DIVIDE_512                                       9
#define PWM2_CLOCK_DIVIDE_1024                                      10
#define PWM2_CLOCK_DIVIDE_2048                                      11
#define PWM2_CLOCK_DIVIDE_4096                                      12
#define PWM2_CLOCK_DIVIDE_8192                                      13
#define PWM2_CLOCK_DIVIDE_16384                                     14
#define PWM2_CLOCK_DIVIDE_32768                                     15
#define PWM2_INTERRUPT_STATUS                                       3:3
#define PWM2_INTERRUPT_STATUS_NOT_ACTIVE                            0
#define PWM2_INTERRUPT_STATUS_ACTIVE                                1
#define PWM2_INTERRUPT_CONTROL                                      2:2
#define PWM2_INTERRUPT_CONTROL_DISABLE                              0
#define PWM2_INTERRUPT_CONTROL_ENABLE                               1
#define PWM2_CONTROL                                                0:0
#define PWM2_CONTROL_DISABLE                                        0
#define PWM2_CONTROL_ENABLE                                         1

/* I2C Byte Count */
#define I2C_BYTE_COUNT                                              0x010040
#define I2C_BYTE_COUNT_COUNT                                        3:0

/* I2C Control */
#define I2C_CONTROL                                                 0x010041
#define I2C_CONTROL_REPEAT                                          6:6
#define I2C_CONTROL_REPEAT_DISABLE                                  0
#define I2C_CONTROL_REPEAT_ENABLE                                   1
#define I2C_CONTROL_INTERRUPT                                       5:5
#define I2C_CONTROL_INTERRUPT_NOT_ACKNOWLEDGE                       0
#define I2C_CONTROL_INTERRUPT_ACKNOWLEDGE                           1
#define I2C_CONTROL_INTERRUPT_CONTROL                               4:4
#define I2C_CONTROL_INTERRUPT_CONTROL_ENABLE                        0
#define I2C_CONTROL_INTERRUPT_CONTROL_DISABLE                       1
#define I2C_CONTROL_STATUS                                          2:2
#define I2C_CONTROL_STATUS_STOP                                     0
#define I2C_CONTROL_STATUS_START                                    1
#define I2C_CONTROL_MODE                                            1:1
#define I2C_CONTROL_MODE_STANDARD                                   0
#define I2C_CONTROL_MODE_FAST                                       1
#define I2C_CONTROL_CONTROL                                         0:0
#define I2C_CONTROL_CONTROL_DISABLE                                 0
#define I2C_CONTROL_CONTROL_ENABLE                                  1

/* I2C Status & Reset */
#define I2C_STATUS                                                  0x010042
#define I2C_STATUS_TRANSFER                                         3:3
#define I2C_STATUS_TRANSFER_IN_PROGRESS                             0
#define I2C_STATUS_TRANSFER_COMPLETED                               1
#define I2C_STATUS_ERROR                                            2:2
#define I2C_STATUS_ERROR_NORMAL                                     0
#define I2C_STATUS_ERROR_BUS_ERROR                                  1
#define I2C_STATUS_ERROR_CLEAR                                      0
#define I2C_STATUS_ACKNOWLEDGE                                      1:1
#define I2C_STATUS_ACKNOWLEDGE_NOT_RECEIVED                         0
#define I2C_STATUS_ACKNOWLEDGE_RECEIVED                             1
#define I2C_STATUS_BUSY                                             0:0
#define I2C_STATUS_BUSY_IDLE                                        0
#define I2C_STATUS_BUSY_BUSY                                        1

/* I2C Slave Address */
#define I2C_SLAVE_ADDRESS                                           0x010043
#define I2C_SLAVE_ADDRESS_ADDRESS                                   7:1
#define I2C_SLAVE_ADDRESS_ACCESS                                    0:0
#define I2C_SLAVE_ADDRESS_ACCESS_WRITE                              0
#define I2C_SLAVE_ADDRESS_ACCESS_READ                               1

/* I2C Data */
#define I2C_DATA_0                                                  0x010044
#define I2C_DATA_1                                                  0x010045
#define I2C_DATA_2                                                  0x010046
#define I2C_DATA_3                                                  0x010047
#define I2C_DATA_4                                                  0x010048
#define I2C_DATA_5                                                  0x010049
#define I2C_DATA_6                                                  0x01004A
#define I2C_DATA_7                                                  0x01004B
#define I2C_DATA_8                                                  0x01004C
#define I2C_DATA_9                                                  0x01004D
#define I2C_DATA_10                                                 0x01004E
#define I2C_DATA_11                                                 0x01004F
#define I2C_DATA_12                                                 0x010050
#define I2C_DATA_13                                                 0x010051
#define I2C_DATA_14                                                 0x010052
#define I2C_DATA_15                                                 0x010053


/*****************************************************************************\
 *                                    SSP                                    *
\*****************************************************************************/

/* TBD */


/*****************************************************************************\
 *                                UART / IRDA                                *
\*****************************************************************************/
/*****************************************************************************\
 *                                                                           *
 *     REGISTER DESCRIPTION                                                  *
 *     |========|======|================================|==============|     *
 *     | Offset | DLAB | Register Name                  |   Comment    |     *
 *     |========|======|================================|==============|     *
 *     |  0x00  |  0   | RX Buffer Register             |      RO      |     *
 *     |--------|------|--------------------------------|--------------|     *
 *     |  0x00  |  0   | TX Holding Register            |      WO      |     *
 *     |--------|------|--------------------------------|--------------|     *
 *     |  0x04  |  0   | Interrupt Enable Register      |      R/W     |     *
 *     |--------|------|--------------------------------|--------------|     *
 *     |  0x08  |  X   | Interrupt Identification Reg.  |      RO      |     *
 *     |--------|------|--------------------------------|--------------|     *
 *     |  0x08  |  X   | FIFO Control Register          |      WO      |     *
 *     |--------|------|--------------------------------|--------------|     *
 *     |  0x0C  |  X   | Line Control Register          |      R/W     |     *
 *     |--------|------|--------------------------------|--------------|     *
 *     |  0x10  |  X   | Modem Control Register         |      R/W     |     *
 *     |--------|------|--------------------------------|--------------|     *
 *     |  0x14  |  X   | Line Status Register           |      RO      |     *
 *     |--------|------|--------------------------------|--------------|     *
 *     |  0x18  |  X   | Modem Status Register          | Partially RO |     *
 *     |--------|------|--------------------------------|--------------|     *
 *     |  0x1C  |  X   | Scratch Register               |      R/W     |     *
 *     |--------|------|--------------------------------|--------------|     *
 *     |  0x00  |  1   | Divisor Latch (LSB)            |      R/W     |     *
 *     |--------|------|--------------------------------|--------------|     *
 *     |  0x04  |  1   | Divisor Latch (MSB)            |      R/W     |     *
 *     |--------|------|--------------------------------|--------------|     *
 *     |  0x08  |  1   | Enhanced Feature Register      |              |     *
 *     |--------|------|--------------------------------|              |     *
 *     |  0x0C  |  1   | XON1 Register                  |      R/W     |     *
 *     |--------|------|--------------------------------|     Only     |     *
 *     |  0x10  |  1   | XON2 Register                  |    present   |     *
 *     |--------|------|--------------------------------|     when     |     *
 *     |  0x14  |  1   | XOFF1 Register                 |  LCR = 0xBF  |     *
 *     |--------|------|--------------------------------|              |     *
 *     |  0x18  |  1   | XOFF2 Register                 |              |     *
 *     |========|======|================================|==============|     *
 *                                                                           *
 *     NOTES:                                                                *
 *       X   = Don't Care                                                    *
 *       RO  = Read Only                                                     *
 *       WO  = Write Only                                                    *
 *       R/W = Read/Write                                                    *
 *                                                                           *
\*****************************************************************************/


/*****************************************************************************\
 * UART 0                                                                    *
\*****************************************************************************/

/* UART 0 when DLAB = 0 (normal mode), LCR != 0xBF */

#define UART_0_RECEIVE_BUFFER                           0x030000
#define	UART_0_RECEIVE_BUFFER_DATA						7:0

#define UART_0_TRANSMIT									0x030000
#define UART_0_TRANSMIT_DATA							7:0

#define UART_0_INTERRUPT                                0x030004
#define UART_0_INTERRUPT_CTS                            7:7
#define UART_0_INTERRUPT_CTS_DISABLE                    0
#define UART_0_INTERRUPT_CTS_ENABLE                     1
#define UART_0_INTERRUPT_RTS                            6:6
#define UART_0_INTERRUPT_RTS_DISABLE                    0
#define UART_0_INTERRUPT_RTS_ENABLE                     1
#define UART_0_INTERRUPT_XOFF                           5:5
#define UART_0_INTERRUPT_XOFF_DISABLE                   0
#define UART_0_INTERRUPT_XOFF_ENABLE                    1
#define UART_0_INTERRUPT_MODEM_STATUS                   3:3
#define UART_0_INTERRUPT_MODEM_STATUS_DISABLE           0
#define UART_0_INTERRUPT_MODEM_STATUS_ENABLE            1
#define UART_0_INTERRUPT_LINE_STATUS                    2:2
#define UART_0_INTERRUPT_LINE_STATUS_DISABLE            0
#define UART_0_INTERRUPT_LINE_STATUS_ENABLE             1
#define UART_0_INTERRUPT_TX_EMPTY                       1:1
#define UART_0_INTERRUPT_TX_EMPTY_DISABLE               0
#define UART_0_INTERRUPT_TX_EMPTY_ENABLE                1
#define UART_0_INTERRUPT_RX_BUFFER                      0:0
#define UART_0_INTERRUPT_RX_BUFFER_DISABLE              0
#define UART_0_INTERRUPT_RX_BUFFER_ENABLE               1


/* UART 0 when DLAB = 1 (configuration mode), LCR != 0xBF */

#define	UART_0_DIVISOR_LATCH_LSB						0x030000
#define	UART_0_DIVISOR_LATCH_LSB_VALUE					7:0

#define	UART_0_DIVISOR_LATCH_MSB						0x030004
#define	UART_0_DIVISOR_LATCH_MSB_VALUE					7:0


/* UART 0 when DLAB = X (don't care), LCR != 0xBF */

#define UART_0_STATUS                                   0x030008
#define UART_0_STATUS_FIFO                              7:6
#define UART_0_STATUS_FIFO_DISABLE                      0
#define UART_0_STATUS_FIFO_ENABLE                       3
#define UART_0_STATUS_INTERRUPT                         5:1
#define UART_0_STATUS_INTERRUPT_MODEM_STATUS_CHANGE     0
#define UART_0_STATUS_INTERRUPT_TX_EMPTY                1
#define UART_0_STATUS_INTERRUPT_RX_DATA_RECEIVED        2
#define UART_0_STATUS_INTERRUPT_LINE_STATUS             3
#define UART_0_STATUS_INTERRUPT_RX_CHAR_TIMEOUT         6
#define UART_0_STATUS_INTERRUPT_XOFF_RECEIVED           8
#define UART_0_STATUS_INTERRUPT_CTS_RTS                 16
#define UART_0_STATUS_INTERRUPT_PENDING                 1:1
#define UART_0_STATUS_INTERRUPT_PENDING_YES             0
#define UART_0_STATUS_INTERRUPT_PENDING_NO              1

#define UART_0_FIFO                                     0x030008
#define UART_0_FIFO_RX_TRIGGER                          7:6
#define UART_0_FIFO_RX_TRIGGER_DEFAULT                  0
#define UART_0_FIFO_RX_TRIGGER_QUARTER                  1
#define UART_0_FIFO_RX_TRIGGER_HALF                     2
#define UART_0_FIFO_RX_TRIGGER_MINUS_2                  3
#define UART_0_FIFO_TX_TRIGGER                          5:4
#define UART_0_FIFO_TX_TRIGGER_DEFAULT                  0
#define UART_0_FIFO_TX_TRIGGER_QUARTER                  1
#define UART_0_FIFO_TX_TRIGGER_HALF                     2
#define UART_0_FIFO_TX_TRIGGER_MINUS_2                  3
#define UART_0_FIFO_DMA_MODE                            3:3
#define UART_0_FIFO_DMA_MODE_ZERO                       0
#define UART_0_FIFO_DMA_MODE_ONE                        1
#define UART_0_FIFO_TX_CLEAR                            2:2
#define UART_0_FIFO_TX_CLEAR_NO                         0
#define UART_0_FIFO_TX_CLEAR_CLEAR                      1
#define UART_0_FIFO_RX_CLEAR                            1:1
#define UART_0_FIFO_RX_CLEAR_NO                         0
#define UART_0_FIFO_RX_CLEAR_CLEAR                      1
#define UART_0_FIFO_ENABLE                              0:0
#define UART_0_FIFO_ENABLE_DISABLE                      0
#define UART_0_FIFO_ENABLE_ENABLE                       1

#define UART_0_LINE_CTRL                                0x03000C
#define	UART_0_LINE_CTRL_ENHANCED						7:0
#define	UART_0_LINE_CTRL_ENHANCED_ENABLE				0xBF
#define UART_0_LINE_CTRL_DLAB                           7:7
#define UART_0_LINE_CTRL_DLAB_RESET                     0
#define UART_0_LINE_CTRL_DLAB_SET                       1
#define UART_0_LINE_CTRL_LINE_BREAK                     6:6
#define UART_0_LINE_CTRL_LINE_BREAK_RESET               0
#define UART_0_LINE_CTRL_LINE_BREAK_SET                 1
#define UART_0_LINE_CTRL_PARITY                         5:3
#define UART_0_LINE_CTRL_PARITY_DISABLE                 0
#define UART_0_LINE_CTRL_PARITY_ODD                     1
#define UART_0_LINE_CTRL_PARITY_EVEN                    3
#define UART_0_LINE_CTRL_PARITY_MARK                    5
#define UART_0_LINE_CTRL_PARITY_SPACE                   7
#define UART_0_LINE_CTRL_STOPBITS                       2:2
#define UART_0_LINE_CTRL_STOPBITS_ONE                   0
#define UART_0_LINE_CTRL_STOPBITS_TWO                   1
#define UART_0_LINE_CTRL_CHARSIZE                       1:0
#define UART_0_LINE_CTRL_CHARSIZE_5                     0
#define UART_0_LINE_CTRL_CHARSIZE_6                     1
#define UART_0_LINE_CTRL_CHARSIZE_7                     2
#define UART_0_LINE_CTRL_CHARSIZE_8                     3

#define UART_0_MODEM_CTRL                               0x030010
#define UART_0_MODEM_CTRL_XOFF_STATUS                   7:7
#define UART_0_MODEM_CTRL_XOFF_STATUS_XON               0
#define UART_0_MODEM_CTRL_XOFF_STATUS_XOFF              1
#define UART_0_MODEM_CTRL_INFRARED                      6:6
#define UART_0_MODEM_CTRL_INFRARED_DISABLE              0
#define UART_0_MODEM_CTRL_INFRARED_ENABLE               1
#define UART_0_MODEM_CTRL_LOOPBACK                      4:4
#define UART_0_MODEM_CTRL_LOOPBACK_DISABLE              0
#define UART_0_MODEM_CTRL_LOOPBACK_ENABLE               1
#define UART_0_MODEM_CTRL_OUT2                          3:3
#define UART_0_MODEM_CTRL_OUT2_RESET                    0
#define UART_0_MODEM_CTRL_OUT2_SET                      1
#define UART_0_MODEM_CTRL_OUT1                          2:2
#define UART_0_MODEM_CTRL_OUT1_RESET                    0
#define UART_0_MODEM_CTRL_OUT1_SET                      1
#define UART_0_MODEM_CTRL_RTS                           1:1
#define UART_0_MODEM_CTRL_RTS_RESET                     0
#define UART_0_MODEM_CTRL_RTS_SET                       1
#define UART_0_MODEM_CTRL_DTR                           0:0
#define UART_0_MODEM_CTRL_DTR_RESET                     0
#define UART_0_MODEM_CTRL_DTR_SET                       1

#define UART_0_LINE_STATUS                              0x030014
#define UART_0_LINE_STATUS_RX_FIFO                      7:7
#define UART_0_LINE_STATUS_RX_FIFO_NO_ERROR             0
#define UART_0_LINE_STATUS_RX_FIFO_ERROR                1
#define UART_0_LINE_STATUS_TRANSMITTER                  6:6
#define UART_0_LINE_STATUS_TRANSMITTER_NOT_EMPTY        0
#define UART_0_LINE_STATUS_TRANSMITTER_EMPTY            1
#define UART_0_LINE_STATUS_TX_HOLDING                   5:5
#define UART_0_LINE_STATUS_TX_HOLDING_NOT_EMPTY         0
#define UART_0_LINE_STATUS_TX_HOLDING_EMPTY             1
#define UART_0_LINE_STATUS_BREAK                        4:4
#define UART_0_LINE_STATUS_BREAK_NO_BREAK               0
#define UART_0_LINE_STATUS_BREAK_BREAK                  1
#define UART_0_LINE_STATUS_FRAME                        3:3
#define UART_0_LINE_STATUS_FRAME_NO_ERROR               0
#define UART_0_LINE_STATUS_FRAME_ERROR                  1
#define UART_0_LINE_STATUS_PARITY                       2:2
#define UART_0_LINE_STATUS_PARITY_NO_ERROR              0
#define UART_0_LINE_STATUS_PARITY_ERROR                 1
#define UART_0_LINE_STATUS_OVERRUN                      1:1
#define UART_0_LINE_STATUS_OVERRUN_NO_ERROR             0
#define UART_0_LINE_STATUS_OVERRUN_ERROR                1
#define UART_0_LINE_STATUS_DATA                         0:0
#define UART_0_LINE_STATUS_DATA_NOT_READY               0
#define UART_0_LINE_STATUS_DATA_READY                   1

#define UART_0_MODEM_STATUS                             0x030018
#define UART_0_MODEM_STATUS_DCD                         7:7
#define UART_0_MODEM_STATUS_DCD_NOT_DETECTED            0
#define UART_0_MODEM_STATUS_DCD_DETECTED                1
#define UART_0_MODEM_STATUS_RI                          6:6
#define UART_0_MODEM_STATUS_RI_NOT_DETECTED             0
#define UART_0_MODEM_STATUS_RI_DETECTED                 1
#define UART_0_MODEM_STATUS_DSR                         5:5
#define UART_0_MODEM_STATUS_DSR_NOT_DETECTED            0
#define UART_0_MODEM_STATUS_DSR_DETECTED                1
#define UART_0_MODEM_STATUS_CTS                         4:4
#define UART_0_MODEM_STATUS_CTS_NOT_DETECTED            0
#define UART_0_MODEM_STATUS_CTS_DETECTED                1
#define UART_0_MODEM_STATUS_DELTA_DCD                   3:3
#define UART_0_MODEM_STATUS_DELTA_DCD_NO_CHANGE         0
#define UART_0_MODEM_STATUS_DELTA_DCD_CHANGE            1
#define UART_0_MODEM_STATUS_TRAILING_RI                 2:2
#define UART_0_MODEM_STATUS_TRAILING_RI_NO_CHANGE       0
#define UART_0_MODEM_STATUS_TRAILING_RI_CHANGE          1
#define UART_0_MODEM_STATUS_DELTA_DSR                   1:1
#define UART_0_MODEM_STATUS_DELTA_DSR_NO_CHANGE         0
#define UART_0_MODEM_STATUS_DELTA_DSR_CHANGE            1
#define UART_0_MODEM_STATUS_DELTA_CTS                   0:0
#define UART_0_MODEM_STATUS_DELTA_CTS_NO_CHANGE         0
#define UART_0_MODEM_STATUS_DELTA_CTS_CHANGE            1

#define UART_0_SCRATCH                                  0x03001C
#define	UART_0_SCRATCH_DATA								7:0


/* UART 0 when LCR = 0xBF (DLAB = 1) (configuration mode) */

#define	UART_0_FEATURE									0x030008
#define	UART_0_FEATURE_AUTO_CTS							7:7
#define	UART_0_FEATURE_AUTO_CTS_DISABLE					0
#define	UART_0_FEATURE_AUTO_CTS_ENABLE					1
#define	UART_0_FEATURE_AUTO_RTS							6:6
#define	UART_0_FEATURE_AUTO_RTS_DISABLE					0
#define	UART_0_FEATURE_AUTO_RTS_ENABLE					1
#define	UART_0_FEATURE_ENHANCED							4:4
#define	UART_0_FEATURE_ENHANCED_DISABLE					0
#define	UART_0_FEATURE_ENHANCED_ENABLE					1
#define	UART_0_FEATURE_SW_TX_FLOW_CTRL_XON1				3:3
#define	UART_0_FEATURE_SW_TX_FLOW_CTRL_XON1_DISABLE		0
#define	UART_0_FEATURE_SW_TX_FLOW_CTRL_XON1_ENABLE		1
#define	UART_0_FEATURE_SW_TX_FLOW_CTRL_XON2				2:2
#define	UART_0_FEATURE_SW_TX_FLOW_CTRL_XON2_DISABLE		0
#define	UART_0_FEATURE_SW_TX_FLOW_CTRL_XON2_ENABLE		1
#define	UART_0_FEATURE_SW_RX_FLOW_CTRL_XON1				1:1
#define	UART_0_FEATURE_SW_RX_FLOW_CTRL_XON1_DISABLE		0
#define	UART_0_FEATURE_SW_RX_FLOW_CTRL_XON1_ENABLE		1
#define	UART_0_FEATURE_SW_RX_FLOW_CTRL_XON2				0:0
#define	UART_0_FEATURE_SW_RX_FLOW_CTRL_XON2_DISABLE		0
#define	UART_0_FEATURE_SW_RX_FLOW_CTRL_XON2_ENABLE		1

#define	UART_0_XON1_CHAR								0x030010
#define	UART_0_XON1_CHAR_DATA							7:0

#define	UART_0_XON2_CHAR								0x030014
#define	UART_0_XON2_CHAR_DATA							7:0

#define	UART_0_XOFF1_CHAR								0x030018
#define	UART_0_XOFF1_CHAR_DATA							7:0

#define	UART_0_XOFF2_CHAR								0x03001C
#define	UART_0_XOFF2_CHAR_DATA							7:0


/*****************************************************************************\
 * UART 1                                                                    *
\*****************************************************************************/

/* UART 1 when DLAB = 0 (normal mode), LCR != 0xBF */

#define UART_1_RECEIVE_BUFFER                           0x030020
#define	UART_1_RECEIVE_BUFFER_DATA						7:0

#define UART_1_TRANSMIT									0x030020
#define UART_1_TRANSMIT_DATA							7:0

#define UART_1_INTERRUPT                                0x030024
#define UART_1_INTERRUPT_CTS                            7:7
#define UART_1_INTERRUPT_CTS_DISABLE                    0
#define UART_1_INTERRUPT_CTS_ENABLE                     1
#define UART_1_INTERRUPT_RTS                            6:6
#define UART_1_INTERRUPT_RTS_DISABLE                    0
#define UART_1_INTERRUPT_RTS_ENABLE                     1
#define UART_1_INTERRUPT_XOFF                           5:5
#define UART_1_INTERRUPT_XOFF_DISABLE                   0
#define UART_1_INTERRUPT_XOFF_ENABLE                    1
#define UART_1_INTERRUPT_MODEM_STATUS                   3:3
#define UART_1_INTERRUPT_MODEM_STATUS_DISABLE           0
#define UART_1_INTERRUPT_MODEM_STATUS_ENABLE            1
#define UART_1_INTERRUPT_LINE_STATUS                    2:2
#define UART_1_INTERRUPT_LINE_STATUS_DISABLE            0
#define UART_1_INTERRUPT_LINE_STATUS_ENABLE             1
#define UART_1_INTERRUPT_TX_EMPTY                       1:1
#define UART_1_INTERRUPT_TX_EMPTY_DISABLE               0
#define UART_1_INTERRUPT_TX_EMPTY_ENABLE                1
#define UART_1_INTERRUPT_RX_BUFFER                      0:0
#define UART_1_INTERRUPT_RX_BUFFER_DISABLE              0
#define UART_1_INTERRUPT_RX_BUFFER_ENABLE               1


/* UART 0 when DLAB = 1 (configuration mode), LCR != 0xBF */

#define	UART_1_DIVISOR_LATCH_LSB						0x030020
#define	UART_1_DIVISOR_LATCH_LSB_VALUE					7:0

#define	UART_1_DIVISOR_LATCH_MSB						0x030024
#define	UART_1_DIVISOR_LATCH_MSB_VALUE					7:0


/* UART 1 when DLAB = X (don't care), LCR != 0xBF */

#define UART_1_STATUS                                   0x030028
#define UART_1_STATUS_FIFO                              7:6
#define UART_1_STATUS_FIFO_DISABLE                      0
#define UART_1_STATUS_FIFO_ENABLE                       3
#define UART_1_STATUS_INTERRUPT                         5:1
#define UART_1_STATUS_INTERRUPT_MODEM_STATUS_CHANGE     0
#define UART_1_STATUS_INTERRUPT_TX_EMPTY                1
#define UART_1_STATUS_INTERRUPT_RX_DATA_RECEIVED        2
#define UART_1_STATUS_INTERRUPT_LINE_STATUS             3
#define UART_1_STATUS_INTERRUPT_RX_CHAR_TIMEOUT         6
#define UART_1_STATUS_INTERRUPT_XOFF_RECEIVED           8
#define UART_1_STATUS_INTERRUPT_CTS_RTS                 16
#define UART_1_STATUS_INTERRUPT_PENDING                 1:1
#define UART_1_STATUS_INTERRUPT_PENDING_YES             0
#define UART_1_STATUS_INTERRUPT_PENDING_NO              1

#define UART_1_FIFO                                     0x030028
#define UART_1_FIFO_RX_TRIGGER                          7:6
#define UART_1_FIFO_RX_TRIGGER_DEFAULT                  0
#define UART_1_FIFO_RX_TRIGGER_QUARTER                  1
#define UART_1_FIFO_RX_TRIGGER_HALF                     2
#define UART_1_FIFO_RX_TRIGGER_MINUS_2                  3
#define UART_1_FIFO_TX_TRIGGER                          5:4
#define UART_1_FIFO_TX_TRIGGER_DEFAULT                  0
#define UART_1_FIFO_TX_TRIGGER_QUARTER                  1
#define UART_1_FIFO_TX_TRIGGER_HALF                     2
#define UART_1_FIFO_TX_TRIGGER_MINUS_2                  3
#define UART_1_FIFO_DMA_MODE                            3:3
#define UART_1_FIFO_DMA_MODE_ZERO                       0
#define UART_1_FIFO_DMA_MODE_ONE                        1
#define UART_1_FIFO_TX_CLEAR                            2:2
#define UART_1_FIFO_TX_CLEAR_NO                         0
#define UART_1_FIFO_TX_CLEAR_CLEAR                      1
#define UART_1_FIFO_RX_CLEAR                            1:1
#define UART_1_FIFO_RX_CLEAR_NO                         0
#define UART_1_FIFO_RX_CLEAR_CLEAR                      1
#define UART_1_FIFO_ENABLE                              0:0
#define UART_1_FIFO_ENABLE_DISABLE                      0
#define UART_1_FIFO_ENABLE_ENABLE                       1

#define UART_1_LINE_CTRL                                0x03002C
#define	UART_1_LINE_CTRL_ENHANCED						7:0
#define	UART_1_LINE_CTRL_ENHANCED_ENABLE				0xBF
#define UART_1_LINE_CTRL_DLAB                           7:7
#define UART_1_LINE_CTRL_DLAB_RESET                     0
#define UART_1_LINE_CTRL_DLAB_SET                       1
#define UART_1_LINE_CTRL_LINE_BREAK                     6:6
#define UART_1_LINE_CTRL_LINE_BREAK_RESET               0
#define UART_1_LINE_CTRL_LINE_BREAK_SET                 1
#define UART_1_LINE_CTRL_PARITY                         5:3
#define UART_1_LINE_CTRL_PARITY_DISABLE                 0
#define UART_1_LINE_CTRL_PARITY_ODD                     1
#define UART_1_LINE_CTRL_PARITY_EVEN                    3
#define UART_1_LINE_CTRL_PARITY_MARK                    5
#define UART_1_LINE_CTRL_PARITY_SPACE                   7
#define UART_1_LINE_CTRL_STOPBITS                       2:2
#define UART_1_LINE_CTRL_STOPBITS_ONE                   0
#define UART_1_LINE_CTRL_STOPBITS_TWO                   1
#define UART_1_LINE_CTRL_CHARSIZE                       1:0
#define UART_1_LINE_CTRL_CHARSIZE_5                     0
#define UART_1_LINE_CTRL_CHARSIZE_6                     1
#define UART_1_LINE_CTRL_CHARSIZE_7                     2
#define UART_1_LINE_CTRL_CHARSIZE_8                     3

#define UART_1_MODEM_CTRL                               0x030030
#define UART_1_MODEM_CTRL_XOFF_STATUS                   7:7
#define UART_1_MODEM_CTRL_XOFF_STATUS_XON               0
#define UART_1_MODEM_CTRL_XOFF_STATUS_XOFF              1
#define UART_1_MODEM_CTRL_INFRARED                      6:6
#define UART_1_MODEM_CTRL_INFRARED_DISABLE              0
#define UART_1_MODEM_CTRL_INFRARED_ENABLE               1
#define UART_1_MODEM_CTRL_LOOPBACK                      4:4
#define UART_1_MODEM_CTRL_LOOPBACK_DISABLE              0
#define UART_1_MODEM_CTRL_LOOPBACK_ENABLE               1
#define UART_1_MODEM_CTRL_OUT2                          3:3
#define UART_1_MODEM_CTRL_OUT2_RESET                    0
#define UART_1_MODEM_CTRL_OUT2_SET                      1
#define UART_1_MODEM_CTRL_OUT1                          2:2
#define UART_1_MODEM_CTRL_OUT1_RESET                    0
#define UART_1_MODEM_CTRL_OUT1_SET                      1
#define UART_1_MODEM_CTRL_RTS                           1:1
#define UART_1_MODEM_CTRL_RTS_RESET                     0
#define UART_1_MODEM_CTRL_RTS_SET                       1
#define UART_1_MODEM_CTRL_DTR                           0:0
#define UART_1_MODEM_CTRL_DTR_RESET                     0
#define UART_1_MODEM_CTRL_DTR_SET                       1

#define UART_1_LINE_STATUS                              0x030034
#define UART_1_LINE_STATUS_RX_FIFO                      7:7
#define UART_1_LINE_STATUS_RX_FIFO_NO_ERROR             0
#define UART_1_LINE_STATUS_RX_FIFO_ERROR                1
#define UART_1_LINE_STATUS_TRANSMITTER                  6:6
#define UART_1_LINE_STATUS_TRANSMITTER_NOT_EMPTY        0
#define UART_1_LINE_STATUS_TRANSMITTER_EMPTY            1
#define UART_1_LINE_STATUS_TX_HOLDING                   5:5
#define UART_1_LINE_STATUS_TX_HOLDING_NOT_EMPTY         0
#define UART_1_LINE_STATUS_TX_HOLDING_EMPTY             1
#define UART_1_LINE_STATUS_BREAK                        4:4
#define UART_1_LINE_STATUS_BREAK_NO_BREAK               0
#define UART_1_LINE_STATUS_BREAK_BREAK                  1
#define UART_1_LINE_STATUS_FRAME                        3:3
#define UART_1_LINE_STATUS_FRAME_NO_ERROR               0
#define UART_1_LINE_STATUS_FRAME_ERROR                  1
#define UART_1_LINE_STATUS_PARITY                       2:2
#define UART_1_LINE_STATUS_PARITY_NO_ERROR              0
#define UART_1_LINE_STATUS_PARITY_ERROR                 1
#define UART_1_LINE_STATUS_OVERRUN                      1:1
#define UART_1_LINE_STATUS_OVERRUN_NO_ERROR             0
#define UART_1_LINE_STATUS_OVERRUN_ERROR                1
#define UART_1_LINE_STATUS_DATA                         0:0
#define UART_1_LINE_STATUS_DATA_NOT_READY               0
#define UART_1_LINE_STATUS_DATA_READY                   1

#define UART_1_MODEM_STATUS                             0x030038
#define UART_1_MODEM_STATUS_DCD                         7:7
#define UART_1_MODEM_STATUS_DCD_NOT_DETECTED            0
#define UART_1_MODEM_STATUS_DCD_DETECTED                1
#define UART_1_MODEM_STATUS_RI                          6:6
#define UART_1_MODEM_STATUS_RI_NOT_DETECTED             0
#define UART_1_MODEM_STATUS_RI_DETECTED                 1
#define UART_1_MODEM_STATUS_DSR                         5:5
#define UART_1_MODEM_STATUS_DSR_NOT_DETECTED            0
#define UART_1_MODEM_STATUS_DSR_DETECTED                1
#define UART_1_MODEM_STATUS_CTS                         4:4
#define UART_1_MODEM_STATUS_CTS_NOT_DETECTED            0
#define UART_1_MODEM_STATUS_CTS_DETECTED                1
#define UART_1_MODEM_STATUS_DELTA_DCD                   3:3
#define UART_1_MODEM_STATUS_DELTA_DCD_NO_CHANGE         0
#define UART_1_MODEM_STATUS_DELTA_DCD_CHANGE            1
#define UART_1_MODEM_STATUS_TRAILING_RI                 2:2
#define UART_1_MODEM_STATUS_TRAILING_RI_NO_CHANGE       0
#define UART_1_MODEM_STATUS_TRAILING_RI_CHANGE          1
#define UART_1_MODEM_STATUS_DELTA_DSR                   1:1
#define UART_1_MODEM_STATUS_DELTA_DSR_NO_CHANGE         0
#define UART_1_MODEM_STATUS_DELTA_DSR_CHANGE            1
#define UART_1_MODEM_STATUS_DELTA_CTS                   0:0
#define UART_1_MODEM_STATUS_DELTA_CTS_NO_CHANGE         0
#define UART_1_MODEM_STATUS_DELTA_CTS_CHANGE            1

#define UART_1_SCRATCH                                  0x03003C
#define	UART_1_SCRATCH_DATA								7:0


/* UART 1 when LCR = 0xBF (DLAB = 1) (configuration mode) */

#define	UART_1_FEATURE									0x030028
#define	UART_1_FEATURE_AUTO_CTS							7:7
#define	UART_1_FEATURE_AUTO_CTS_DISABLE					0
#define	UART_1_FEATURE_AUTO_CTS_ENABLE					1
#define	UART_1_FEATURE_AUTO_RTS							6:6
#define	UART_1_FEATURE_AUTO_RTS_DISABLE					0
#define	UART_1_FEATURE_AUTO_RTS_ENABLE					1
#define	UART_1_FEATURE_ENHANCED							4:4
#define	UART_1_FEATURE_ENHANCED_DISABLE					0
#define	UART_1_FEATURE_ENHANCED_ENABLE					1
#define	UART_1_FEATURE_SW_TX_FLOW_CTRL_XON1				3:3
#define	UART_1_FEATURE_SW_TX_FLOW_CTRL_XON1_DISABLE		0
#define	UART_1_FEATURE_SW_TX_FLOW_CTRL_XON1_ENABLE		1
#define	UART_1_FEATURE_SW_TX_FLOW_CTRL_XON2				2:2
#define	UART_1_FEATURE_SW_TX_FLOW_CTRL_XON2_DISABLE		0
#define	UART_1_FEATURE_SW_TX_FLOW_CTRL_XON2_ENABLE		1
#define	UART_1_FEATURE_SW_RX_FLOW_CTRL_XON1				1:1
#define	UART_1_FEATURE_SW_RX_FLOW_CTRL_XON1_DISABLE		0
#define	UART_1_FEATURE_SW_RX_FLOW_CTRL_XON1_ENABLE		1
#define	UART_1_FEATURE_SW_RX_FLOW_CTRL_XON2				0:0
#define	UART_1_FEATURE_SW_RX_FLOW_CTRL_XON2_DISABLE		0
#define	UART_1_FEATURE_SW_RX_FLOW_CTRL_XON2_ENABLE		1

#define	UART_1_XON1_CHAR								0x030030
#define	UART_1_XON1_CHAR_DATA							7:0

#define	UART_1_XON2_CHAR								0x030034
#define	UART_1_XON2_CHAR_DATA							7:0

#define	UART_1_XOFF1_CHAR								0x030038
#define	UART_1_XOFF1_CHAR_DATA							7:0

#define	UART_1_XOFF2_CHAR								0x03003C
#define	UART_1_XOFF2_CHAR_DATA							7:0


/*****************************************************************************\
 *                                 USB HOST                                  *
\*****************************************************************************/

/* TBD */


/*****************************************************************************\
 *                                USB SLAVE                                  *
\*****************************************************************************/

/* TBD */


/*****************************************************************************\
 *                            DISPLAY CONTROLLER                             *
\*****************************************************************************/

/* TBD */


/*****************************************************************************\
 *                                 ZV PORT                                   *
\*****************************************************************************/

/* TBD */


/*****************************************************************************\
 *                                AC97 / I2S                                 *
\*****************************************************************************/

/* AC97 TX Slot 0: TAG */
#define AC97_TX_SLOT0                                               0x0A0100
#define AC97_TX_SLOT0_VALID_FRAME_TAG                               15:15
#define AC97_TX_SLOT0_VALID_FRAME_TAG_IGNORE                        0
#define AC97_TX_SLOT0_VALID_FRAME_TAG_INTERPRET                     1
#define AC97_TX_SLOT0_VALID_S1                                      14:14
#define AC97_TX_SLOT0_VALID_S1_NO_DATA                              0
#define AC97_TX_SLOT0_VALID_S1_DATA                                 1
#define AC97_TX_SLOT0_VALID_S2                                      13:13
#define AC97_TX_SLOT0_VALID_S2_NO_DATA                              0
#define AC97_TX_SLOT0_VALID_S2_DATA                                 1
#define AC97_TX_SLOT0_VALID_S3                                      12:12
#define AC97_TX_SLOT0_VALID_S3_NO_DATA                              0
#define AC97_TX_SLOT0_VALID_S3_DATA                                 1
#define AC97_TX_SLOT0_VALID_S4                                      11:11
#define AC97_TX_SLOT0_VALID_S4_NO_DATA                              0
#define AC97_TX_SLOT0_VALID_S4_DATA                                 1

/* AC97 TX Slot 1: Command Address Port */
#define AC97_TX_SLOT1                                               0x0A0104
#define AC97_TX_SLOT1_READ_WRITE                                    19:19
#define AC97_TX_SLOT1_READ_WRITE_WRITE                              0
#define AC97_TX_SLOT1_READ_WRITE_READ                               1
#define AC97_TX_SLOT1_CONTROL_REGISTER_INDEX                        18:12

/* AC97 TX Slot 2: Command Address Port */
#define AC97_TX_SLOT2                                               0x0A0108
#define AC97_TX_SLOT2_DATA                                          19:4

/* AC97 TX Slot 3: PCM Playback Left Channel */
#define AC97_TX_SLOT3                                               0x0A010C
#define AC97_TX_SLOT3_DATA                                          19:0

/* AC97 TX Slot 4: PCM Playback Right Channel */
#define AC97_TX_SLOT4                                               0x0A0110
#define AC97_TX_SLOT4_DATA                                          19:0

/* AC97 RX Slot 0: TAG */
#define AC97_RX_SLOT0                                               0x0A0140
#define AC97_RX_SLOT0_CODEC_READY_FLAG                              15:15
#define AC97_RX_SLOT0_CODEC_READY_FLAG_IGNORE                       0
#define AC97_RX_SLOT0_CODEC_READY_FLAG_READY                        1
#define AC97_RX_SLOT0_VALID_S1                                      14:14
#define AC97_RX_SLOT0_VALID_S1_NO_DATA                              0
#define AC97_RX_SLOT0_VALID_S1_DATA                                 1
#define AC97_RX_SLOT0_VALID_S2                                      13:13
#define AC97_RX_SLOT0_VALID_S2_NO_DATA                              0
#define AC97_RX_SLOT0_VALID_S2_DATA                                 1
#define AC97_RX_SLOT0_VALID_S3                                      12:12
#define AC97_RX_SLOT0_VALID_S3_NO_DATA                              0
#define AC97_RX_SLOT0_VALID_S3_DATA                                 1
#define AC97_RX_SLOT0_VALID_S4                                      11:11
#define AC97_RX_SLOT0_VALID_S4_NO_DATA                              0
#define AC97_RX_SLOT0_VALID_S4_DATA                                 1

/* AC97 RX Slot 1: Status Address Port */
#define AC97_RX_SLOT1                                               0x0A0144
#define AC97_RX_SLOT1_CONTROL_REGISTER_INDEX                        18:12
#define AC97_RX_SLOT1_ON_DEMAND_DATA_S3                             11:11
#define AC97_RX_SLOT1_ON_DEMAND_DATA_S3_SEND                        0
#define AC97_RX_SLOT1_ON_DEMAND_DATA_S3_DO_NOT_SEND                 1
#define AC97_RX_SLOT1_ON_DEMAND_DATA_S4                             10:10
#define AC97_RX_SLOT1_ON_DEMAND_DATA_S4_SEND                        0
#define AC97_RX_SLOT1_ON_DEMAND_DATA_S4_DO_NOT_SEND                 1

/* AC97 RX Slot 2: Status Address Port */
#define AC97_RX_SLOT2                                               0x0A0148
#define AC97_RX_SLOT2_DATA                                          19:4

/* AC97 RX Slot 3: PCM Record Left Channel */
#define AC97_RX_SLOT3                                               0x0A014C
#define AC97_RX_SLOT3_DATA                                          19:0

/* AC97 RX Slot 4: PCM Record Right Channel */
#define AC97_RX_SLOT4                                               0x0A0150
#define AC97_RX_SLOT4_DATA                                          19:0

/* AC97 Control & Status */
#define AC97_CONTROL_STATUS                                         0x0A0180
#define AC97_CONTROL_STATUS_DATA_DROPPED_COUNT                      15:10
#define AC97_CONTROL_STATUS_SYNC_CONTROL                            9:9
#define AC97_CONTROL_STATUS_SYNC_CONTROL_ENABLE                     0
#define AC97_CONTROL_STATUS_SYNC_CONTROL_STOP                       1
#define AC97_CONTROL_STATUS_CLOCK_CONTROL                           8:8
#define AC97_CONTROL_STATUS_CLOCK_CONTROL_NOT_RUNNING               0
#define AC97_CONTROL_STATUS_CLOCK_CONTROL_RUNNING                   1
#define AC97_CONTROL_STATUS_WAKEUP_REQUEST                          7:7
#define AC97_CONTROL_STATUS_WAKEUP_REQUEST_DISABLE                  0
#define AC97_CONTROL_STATUS_WAKEUP_REQUEST_ENABLE                   1
#define AC97_CONTROL_STATUS_STATUS                                  5:4
#define AC97_CONTROL_STATUS_STATUS_OFF                              0
#define AC97_CONTROL_STATUS_STATUS_RESET                            1
#define AC97_CONTROL_STATUS_STATUS_WAITING                          2
#define AC97_CONTROL_STATUS_STATUS_ACTIVE                           3
#define AC97_CONTROL_STATUS_WAKEUP_INTERRUPT                        3:3
#define AC97_CONTROL_STATUS_WAKEUP_INTERRUPT_DISABLED               0
#define AC97_CONTROL_STATUS_WAKEUP_INTERRUPT_ENABLED                1
#define AC97_CONTROL_STATUS_WARM_RESET                              2:2
#define AC97_CONTROL_STATUS_WARM_RESET_NORMAL                       0
#define AC97_CONTROL_STATUS_WARM_RESET_RESET                        1
#define AC97_CONTROL_STATUS_COLD_RESET                              1:1
#define AC97_CONTROL_STATUS_COLD_RESET_NORMAL                       0
#define AC97_CONTROL_STATUS_COLD_RESET_RESET                        1
#define AC97_CONTROL_STATUS_CONTROL                                 0:0
#define AC97_CONTROL_STATUS_CONTROL_DISABLED                        0
#define AC97_CONTROL_STATUS_CONTROL_ENABLED                         1

/* I2S TX Data Left */
#define I2S_TX_DATA_LEFT                                            0x0A0200
#define I2S_TX_DATA_LEFT_DATA                                       15:0

/* I2S TX Data Right */
#define I2S_TX_DATA_RIGHT                                           0x0A0204
#define I2S_TX_DATA_RIGHT_DATA                                      15:0

/* I2S RX Data Left */
#define I2S_RX_DATA_LEFT                                            0x0A0208
#define I2S_RX_DATA_LEFT_DATA                                       15:0

/* I2S RX Data Right */
#define I2S_RX_DATA_RIGHT                                           0x0A020C
#define I2S_RX_DATA_RIGHT_DATA                                      15:0

/* I2S Control & Status */
#define I2S_CONTROL_STATUS                                          0x0A0210
#define I2S_CONTROL_STATUS_RECEIVE_STATUS                           11:11
#define I2S_CONTROL_STATUS_RECEIVE_STATUS_NO_ERROR                  0
#define I2S_CONTROL_STATUS_RECEIVE_STATUS_OVERFLOW                  1
#define I2S_CONTROL_STATUS_TRANSMIT_STATUS                          10:10
#define I2S_CONTROL_STATUS_TRANSMIT_STATUS_NO_ERROR                 0
#define I2S_CONTROL_STATUS_TRANSMIT_STATUS_UNDERFLOW                1
#define I2S_CONTROL_STATUS_CONTROL                                  2:2
#define I2S_CONTROL_STATUS_CONTROL_DISABLED                         0
#define I2S_CONTROL_STATUS_CONTROL_ENABLED                          1

/* I2S Clock Control */
#define I2S_CLOCK_CONTROL                                           0x0A0214
#define I2S_CLOCK_CONTROL_MODE_SELECT                               7:7
#define I2S_CLOCK_CONTROL_MODE_SELECT_SLAVE                         0
#define I2S_CLOCK_CONTROL_MODE_SELECT_MASTER                        1
#define I2S_CLOCK_CONTROL_WORD_SELECT                               6:5
#define I2S_CLOCK_CONTROL_WORD_SELECT_16BIT                         0
#define I2S_CLOCK_CONTROL_WORD_SELECT_24BIT                         1
#define I2S_CLOCK_CONTROL_WORD_SELECT_32BIT                         2
#define I2S_CLOCK_CONTROL_CLOCK_DIVIDER                             4:0

/*****************************************************************************\
 *                                    8051                                   *
\*****************************************************************************/

/* Reset */
#define U8051_RESET                                                 0x0B0000
#define U8051_RESET_CONTROL                                         0:0
#define U8051_RESET_CONTROL_RESET                                   0
#define U8051_RESET_CONTROL_ENABLE                                  1

/* Mode Select */
#define U8051_MODE_SELECT                                           0x0B0004
#define U8051_MODE_SELECT_USB_SLAVE_WAIT_STATE                      7:6
#define U8051_MODE_SELECT_USB_SLAVE_WAIT_STATE_NONE                 0
#define U8051_MODE_SELECT_USB_SLAVE_WAIT_STATE_1WS                  1
#define U8051_MODE_SELECT_USB_SLAVE_WAIT_STATE_2WS                  2
#define U8051_MODE_SELECT_SRAM                                      5:5
#define U8051_MODE_SELECT_SRAM_ENABLED                              0
#define U8051_MODE_SELECT_SRAM_DISABLED                             1
#define U8051_MODE_SELECT_EXTERNAL_INTERFACE                        4:4
#define U8051_MODE_SELECT_EXTERNAL_INTERFACE_12BIT                  0
#define U8051_MODE_SELECT_EXTERNAL_INTERFACE_8BIT                   1
#define U8051_MODE_SELECT_TEST_MODE                                 3:3
#define U8051_MODE_SELECT_TEST_MODE_NORMAL                          0
#define U8051_MODE_SELECT_TEST_MODE_ENABLED                         1
#define U8051_MODE_SELECT_CODEC_SELECT                              2:2
#define U8051_MODE_SELECT_CODEC_SELECT_AC97                         0
#define U8051_MODE_SELECT_CODEC_SELECT_I2S                          1
#define U8051_MODE_SELECT_DIVIDER                                   1:0
#define U8051_MODE_SELECT_DIVIDER_2                                 0
#define U8051_MODE_SELECT_DIVIDER_3                                 1
#define U8051_MODE_SELECT_DIVIDER_4                                 2
#define U8051_MODE_SELECT_DIVIDER_5                                 3

/* 8051 Protocol Interrupt */
#define U8051_8051_PROTOCOL_INTERRUPT                               0x0B0008
#define U8051_8051_PROTOCOL_INTERRUPT_TOKEN                         31:0

/* CPU Protocol Interrupt */
#define U8051_CPU_PROTOCOL_INTERRUPT                                0x0B000C
#define U8051_CPU_PROTOCOL_INTERRUPT_TOKEN                          31:0

/* SRAM Address Space */
#define U8051_SRAM                                                  0x0C0000
#define U8051_SRAM_SIZE                                             0x004000


/*****************************************************************************\
 *                                     DMA                                   *
\*****************************************************************************/

/* DMA 0 SDRAM Address */
#define DMA0_SDRAM                                                  0x0D0000
#define DMA0_SDRAM_MEMORY                                           27:26
#define DMA0_SDRAM_MEMORY_LOCAL                                     0
#define DMA0_SDRAM_MEMORY_EXTERNAL_CS0                              2
#define DMA0_SDRAM_MEMORY_EXTERNAL_CS1                              3
#define DMA0_SDRAM_ADDRESS                                          25:0

/* DMA 0 SRAM Address */
#define DMA0_SRAM                                                   0x0D0004
#define DMA0_SRAM_ADDRESS                                           15:0

/* DMA 0 Size & Control */
#define DMA0_SIZE_CONTROL                                           0x0D0008
#define DMA0_SIZE_CONTROL_ACTIVE                                    31:31
#define DMA0_SIZE_CONTROL_ACTIVE_IDLE                               0
#define DMA0_SIZE_CONTROL_ACTIVE_START                              1
#define DMA0_SIZE_CONTROL_DIRECTION                                 30:30
#define DMA0_SIZE_CONTROL_DIRECTION_WRITE_SRAM                      0
#define DMA0_SIZE_CONTROL_DIRECTION_READ_SRAM                       1
#define DMA0_SIZE_CONTROL_SIZE                                      15:0

/* DMA 1 Source Address */
#define DMA1_SOURCE                                                 0x0D0010
#define DMA1_SOURCE_MEMORY                                          27:26
#define DMA1_SOURCE_MEMORY_LOCAL                                    0
#define DMA1_SOURCE_MEMORY_EXTERNAL_CS0                             2
#define DMA1_SOURCE_MEMORY_EXTERNAL_CS1                             3
#define DMA1_SOURCE_ADDRESS                                         25:0

/* DMA 1 Destination Address */
#define DMA1_DESTINATION                                            0x0D0014
#define DMA1_DESTINATION_MEMORY                                     27:26
#define DMA1_DESTINATION_MEMORY_LOCAL                               0
#define DMA1_DESTINATION_MEMORY_EXTERNAL_CS0                        2
#define DMA1_DESTINATION_MEMORY_EXTERNAL_CS1                        3
#define DMA1_DESTINATION_ADDRESS                                    25:0

/* DMA 1 Size & Control */
#define DMA1_SIZE_CONTROL                                           0x0D0018
#define DMA1_SIZE_CONTROL_ACTIVE                                    31:31
#define DMA1_SIZE_CONTROL_ACTIVE_IDLE                               0
#define DMA1_SIZE_CONTROL_ACTIVE_START                              1
#define DMA1_SIZE_CONTROL_SIZE                                      23:0

/* DMA Abort & Interrupt */
#define DMA_ABORT_INTERRUPT                                         0x0D0020
#define DMA_ABORT_INTERRUPT_ABORT_1                                 5:5
#define DMA_ABORT_INTERRUPT_ABORT_1_ENABLE                          0
#define DMA_ABORT_INTERRUPT_ABORT_1_ABORT                           1
#define DMA_ABORT_INTERRUPT_ABORT_0                                 4:4
#define DMA_ABORT_INTERRUPT_ABORT_0_ENABLE                          0
#define DMA_ABORT_INTERRUPT_ABORT_0_ABORT                           1
#define DMA_ABORT_INTERRUPT_INTERRUPT_STATUS_1                      1:1
#define DMA_ABORT_INTERRUPT_INTERRUPT_STATUS_1_NOT_ACTIVE           0
#define DMA_ABORT_INTERRUPT_INTERRUPT_STATUS_1_ACTIVE               1
#define DMA_ABORT_INTERRUPT_INTERRUPT_STATUS_0                      0:0
#define DMA_ABORT_INTERRUPT_INTERRUPT_STATUS_0_NOT_ACTIVE           0
#define DMA_ABORT_INTERRUPT_INTERRUPT_STATUS_0_ACTIVE               1


/*****************************************************************************\
 *                                  2D ENGINE                                *
\*****************************************************************************/

/* 2D Source */
#define DE_SOURCE                                                   0x100000
#define DE_SOURCE_WRAP                                              31:31
#define DE_SOURCE_WRAP_DISABLE                                      0
#define DE_SOURCE_WRAP_ENABLE                                       1
#define DE_SOURCE_X_K1                                              29:16
#define DE_SOURCE_Y_K2                                              15:0

/* 2D Destination */
#define DE_DESTINATION                                              0x100004
#define DE_DESTINATION_WRAP                                         31:31
#define DE_DESTINATION_WRAP_DISABLE                                 0
#define DE_DESTINATION_WRAP_ENABLE                                  1
#define DE_DESTINATION_X                                            28:16
#define DE_DESTINATION_Y                                            15:0

/* 2D Dimension */
#define DE_DIMENSION                                                0x100008
#define DE_DIMENSION_X_VL                                           28:16
#define DE_DIMENSION_Y_ET                                           15:0

/* 2D Control */
#define DE_CONTROL                                                  0x10000C
#define DE_CONTROL_STATUS                                           31:31
#define DE_CONTROL_STATUS_STOP                                      0
#define DE_CONTROL_STATUS_START                                     1
#define DE_CONTROL_PATTERN_SELECT                                   30:30
#define DE_CONTROL_PATTERN_SELECT_MONOCHROME                        0
#define DE_CONTROL_PATTERN_SELECT_COLOR                             1
#define DE_CONTROL_UPDATE_DESTINATION_X                             29:29
#define DE_CONTROL_UPDATE_DESTINATION_X_DISABLE                     0
#define DE_CONTROL_UPDATE_DESTINATION_X_ENABLE                      1
#define DE_CONTROL_QUICK_START_CONTROL                              28:28
#define DE_CONTROL_QUICK_START_CONTROL_DISABLE                      0
#define DE_CONTROL_QUICK_START_CONTROL_ENABLE                       1
#define DE_CONTROL_DIRECTION_CONTROL                                27:27
#define DE_CONTROL_DIRECTION_CONTROL_LEFT_TO_RIGHT                  0
#define DE_CONTROL_DIRECTION_CONTROL_RIGHT_TO_LEFT                  1
#define DE_CONTROL_MAJOR_AXIS                                       26:26
#define DE_CONTROL_MAJOR_AXIS_X                                     0
#define DE_CONTROL_MAJOR_AXIS_Y                                     1
#define DE_CONTROL_X_STEP                                           25:25
#define DE_CONTROL_X_STEP_POSITIVE                                  0
#define DE_CONTROL_X_STEP_NEGATIVE                                  1
#define DE_CONTROL_Y_STEP                                           24:24
#define DE_CONTROL_Y_STEP_POSITIVE                                  0
#define DE_CONTROL_Y_STEP_NEGATIVE                                  1
#define DE_CONTROL_STRETCH                                          23:23
#define DE_CONTROL_STRETCH_DISABLE                                  0
#define DE_CONTROL_STRETCH_ENABLE                                   1
#define DE_CONTROL_HOST_BITBLT                                      22:22
#define DE_CONTROL_HOST_BITBLT_COLOR                                0
#define DE_CONTROL_HOST_BITBLT_MONOCHROME                           1
#define DE_CONTROL_DRAW_LAST_PIXEL                                  21:21
#define DE_CONTROL_DRAW_LAST_PIXEL_DONOT_DRAW                       0
#define DE_CONTROL_DRAW_LAST_PIXEL_DRAW                             1
#define DE_CONTROL_COMMAND                                          20:16
#define DE_CONTROL_COMMAND_BITBLT                                   0
#define DE_CONTROL_COMMAND_RECTANGLE_FILL                           1
#define DE_CONTROL_COMMAND_DETILE                                   2
#define DE_CONTROL_COMMAND_TRAPEZOID_FILL                           3
#define DE_CONTROL_COMMAND_ALPHA_BLEND                              4
#define DE_CONTROL_COMMAND_RLE_STRIP                                5
#define DE_CONTROL_COMMAND_SHORT_STROKE                             6
#define DE_CONTROL_COMMAND_LINE_DRAW                                7
#define DE_CONTROL_COMMAND_HOST_WRITE                               8
#define DE_CONTROL_COMMAND_HOST_READ                                9
#define DE_CONTROL_COMMAND_HOST_WRITE_BOTTOM_UP                     10
#define DE_CONTROL_COMMAND_ROTATE                                   11
#define DE_CONTROL_COMMAND_FONT                                     12
#define DE_CONTROL_COMMAND_TEXTURE_LOAD                             13
#define DE_CONTROL_ROP_CONTROL                                      15:15
#define DE_CONTROL_ROP_CONTROL_ROP2                                 0
#define DE_CONTROL_ROP_CONTROL_ROP3                                 1
#define DE_CONTROL_ROP2_CONTROL                                     14:14
#define DE_CONTROL_ROP2_CONTROL_BITMAP                              0
#define DE_CONTROL_ROP2_CONTROL_PATTERN                             1
#define DE_CONTROL_MONOCHROME_PACK_CONTROL                          13:12
#define DE_CONTROL_MONOCHROME_PACK_CONTROL_NOT_PACKED               0
#define DE_CONTROL_MONOCHROME_PACK_CONTROL_8_BIT                    1
#define DE_CONTROL_MONOCHROME_PACK_CONTROL_16_BIT                   2
#define DE_CONTROL_MONOCHROME_PACK_CONTROL_32_BIT                   3
#define DE_CONTROL_REPEAT_ROTATION                                  11:11
#define DE_CONTROL_REPEAT_ROTATION_DISABLE                          0
#define DE_CONTROL_REPEAT_ROTATION_ENABLE                           1
#define DE_CONTROL_TRANSPARENCY_MATCH_SELECT                        10:10
#define DE_CONTROL_TRANSPARENCY_MATCH_SELECT_OPAQUE                 0
#define DE_CONTROL_TRANSPARENCY_MATCH_SELECT_TRANSPARENT            1
#define DE_CONTROL_TRANSPARENCY_SELECT                              9:9
#define DE_CONTROL_TRANSPARENCY_SELECT_SOURCE                       0
#define DE_CONTROL_TRANSPARENCY_SELECT_DESTINATION                  1
#define DE_CONTROL_TRANSPARENCY_CONTROL                             8:8
#define DE_CONTROL_TRANSPARENCY_CONTROL_DISABLE                     0
#define DE_CONTROL_TRANSPARENCY_CONTROL_ENABLE                      1
#define DE_CONTROL_ROP                                              7:0

/* 2D Pitch */
#define DE_PITCH                                                    0x100010
#define DE_PITCH_DESTINATION                                        28:16
#define DE_PITCH_SOURCE                                             12:0

/* 2D Foreground */
#define DE_FOREGROUND                                               0x100014

/* 2D Background */
#define DE_BACKGROUND                                               0x100018

/* 2D Stretch & Format */
#define DE_STRETCH_FORMAT                                           0x10001C
#define DE_STRETCH_FORMAT_PATTERN_XY                                30:30
#define DE_STRETCH_FORMAT_PATTERN_XY_LINEAR                         0
#define DE_STRETCH_FORMAT_PATTERN_XY_LINEAR_AND_XY                  1
#define DE_STRETCH_FORMAT_PATTERN_Y_ORIGIN                          29:27
#define DE_STRETCH_FORMAT_PATTERN_X_ORIGIN                          25:23
#define DE_STRETCH_FORMAT_FORMAT                                    21:20
#define DE_STRETCH_FORMAT_FORMAT_8BPP                               0
#define DE_STRETCH_FORMAT_FORMAT_16BPP                              1
#define DE_STRETCH_FORMAT_FORMAT_32BPP                              2
#define DE_STRETCH_FORMAT_ADRESSING                                 19:16
#define DE_STRETCH_FORMAT_ADRESSING_XY                              0
#define DE_STRETCH_FORMAT_ADRESSING_LINEAR                          15
#define DE_STRETCH_FORMAT_HEIGHT                                    11:0

/* 2D Color Compare */
#define DE_COLOR_COMPARE                                            0x100020
#define DE_COLOR_COMPARE_COLOR                                      23:0

/* 2D Color Compare Mask */
#define DE_COLOR_COMPARE_MASK                                       0x100024
#define DE_COLOR_COMPARE_MASK_MASK                                  23:0

/* 2D Mask */
#define DE_MASK                                                     0x100028
#define DE_MASK_BYTE                                                31:16
#define DE_MASK_BIT                                                 15:0

/* 2D Clip TL */
#define DE_CLIP_TL                                                  0x10002C
#define DE_CLIP_TL_TOP                                              31:16
#define DE_CLIP_TL_CONTROL                                          13:13
#define DE_CLIP_TL_CONTROL_DISABLE                                  0
#define DE_CLIP_TL_CONTROL_ENABLE                                   
#define DE_CLIP_TL_SELECT                                           12:12
#define DE_CLIP_TL_SELECT_INTERIOR                                  0
#define DE_CLIP_TL_SELECT_EXTERIOR                                  1
#define DE_CLIP_TL_LEFT                                             11:0

/* 2D Clip BR */
#define DE_CLIP_BR                                                  0x100030
#define DE_CLIP_BR_BOTTOM                                           31:16
#define DE_CLIP_BR_RIGHT                                            12:0

/* 2D Mono Pattern Low */
#define DE_MONO_PATTERN_LOW                                         0x100034

/* 2D Mono Pattern High */
#define DE_MONO_PATTERN_HIGH                                        0x100038

/* 2D Window Width */
#define DE_WINDOW_WIDTH                                             0x10003C
#define DE_WINDOW_WIDTH_DESTINATION                                 28:16
#define DE_WINDOW_WIDTH_SOURCE                                      12:0

/* 2D Source Base */
#define DE_SOURCE_BASE                                              0x100040
#define DE_SOURCE_BASE_MEMORY                                       27:26
#define DE_SOURCE_BASE_MEMORY_LOCAL                                 0
#define DE_SOURCE_BASE_MEMORY_EXTERNAL_CS0                          2
#define DE_SOURCE_BASE_MEMORY_EXTERNAL_CS1                          3
#define DE_SOURCE_BASE_ADDRESS                                      25:0

/* 2D Destination Base */
#define DE_DESTINATION_BASE                                         0x100044
#define DE_DESTINATION_BASE_MEMORY                                  27:26
#define DE_DESTINATION_BASE_MEMORY_LOCAL                            0
#define DE_DESTINATION_BASE_MEMORY_EXTERNAL_CS0                     2
#define DE_DESTINATION_BASE_MEMORY_EXTERNAL_CS1                     3
#define DE_DESTINATION_BASE_ADDRESS                                 25:0

/* 2D Alpha */
#define DE_ALPHA                                                    0x100048
#define DE_ALPHA_ALPHA                                              7:0

/* 2D Wrap */
#define DE_WRAP                                                     0x10004C
#define DE_WRAP_WIDTH                                               31:16
#define DE_WRAP_HEIGHT                                              15:0

/* 2D Status */
#define DE_STATUS                                                   0x100050
#define DE_STATUS_CSC                                               1:1
#define DE_STATUS_CSC_NO_INTERRUPT                                  0
#define DE_STATUS_CSC_INTERRUPT                                     1
#define DE_STATUS_2D                                                0:0
#define DE_STATUS_2D_NO_INTERRUPT                                   0
#define DE_STATUS_2D_INTERRUPT                                      1

/* CSC Y Source Base */
#define CSC_Y_SOURCE_BASE                                           0x1000C8
#define CSC_Y_SOURCE_BASE_MEMORY                                    27:26
#define CSC_Y_SOURCE_BASE_MEMORY_LOCAL                              0
#define CSC_Y_SOURCE_BASE_MEMORY_EXTERNAL_CS0                       2
#define CSC_Y_SOURCE_BASE_MEMORY_EXTERNAL_CS1                       3
#define CSC_Y_SOURCE_BASE_ADDRESS                                   25:0

/* CSC Constants */
#define CSC_CONSTANTS                                               0x1000CC
#define CSC_CONSTANTS_Y                                             31:24
#define CSC_CONSTANTS_R                                             23:16
#define CSC_CONSTANTS_G                                             15:8
#define CSC_CONSTANTS_B                                             7:0

/* CSC Y Source X */
#define CSC_Y_SOURCE_X                                              0x1000D0
#define CSC_Y_SOURCE_X_INTEGER                                      27:16
#define CSC_Y_SOURCE_X_FRACTIONAL                                   15:3

/* CSC Y Source Y */
#define CSC_Y_SOURCE_Y                                              0x1000D4
#define CSC_Y_SOURCE_Y_INTEGER                                      27:16
#define CSC_Y_SOURCE_Y_FRACTIONAL                                   15:3

/* CSC U Source Base */
#define CSC_U_SOURCE_BASE                                           0x1000D8
#define CSC_U_SOURCE_BASE_MEMORY                                    27:26
#define CSC_U_SOURCE_BASE_MEMORY_LOCAL                              0
#define CSC_U_SOURCE_BASE_MEMORY_EXTERNAL_CS0                       2
#define CSC_U_SOURCE_BASE_MEMORY_EXTERNAL_CS1                       3
#define CSC_U_SOURCE_BASE_ADDRESS                                   25:0

/* CSC V Source Base */
#define CSC_V_SOURCE_BASE                                           0x1000DC
#define CSC_V_SOURCE_BASE_MEMORY                                    27:26
#define CSC_V_SOURCE_BASE_MEMORY_LOCAL                              0
#define CSC_V_SOURCE_BASE_MEMORY_EXTERNAL_CS0                       2
#define CSC_V_SOURCE_BASE_MEMORY_EXTERNAL_CS1                       3
#define CSC_V_SOURCE_BASE_ADDRESS                                   25:0

/* CSC Source Dimension */
#define CSC_SOURCE_DIMENSION                                        0x1000E0
#define CSC_SOURCE_DIMENSION_WIDTH                                  31:16
#define CSC_SOURCE_DIMENSION_HEIGHT                                 15:0

/* CSC Source Pitch */
#define CSC_SOURCE_PITCH                                            0x1000E4
#define CSC_SOURCE_PITCH_Y                                          31:16
#define CSC_SOURCE_PITCH_UV                                         15:0

/* CSC Destination */
#define CSC_DESTINATION                                             0x1000E8
#define CSC_DESTINATION_WRAP                                        31:31
#define CSC_DESTINATION_WRAP_DISABLE                                0
#define CSC_DESTINATION_WRAP_ENABLE                                 1
#define CSC_DESTINATION_X                                           27:16
#define CSC_DESTINATION_Y                                           11:0

/* CSC Destination Dimension */
#define CSC_DESTINATION_DIMENSION                                   0x1000EC
#define CSC_DESTINATION_DIMENSION_WIDTH                             31:16
#define CSC_DESTINATION_DIMENSION_HEIGHT                            15:0

/* CSC Destination Pitch */
#define CSC_DESTINATION_PITCH                                       0x1000F0
#define CSC_DESTINATION_PITCH_X                                     31:16
#define CSC_DESTINATION_PITCH_Y                                     15:0

/* CSC Scale Factor */
#define CSC_SCALE_FACTOR                                            0x1000F4
#define CSC_SCALE_FACTOR_X                                          31:16
#define CSC_SCALE_FACTOR_Y                                          15:0

/* CSC Destination Base */
#define CSC_DESTINATION_BASE                                        0x1000F8
#define CSC_DESTINATION_BASE_MEMORY                                 27:26
#define CSC_DESTINATION_BASE_MEMORY_LOCAL                           0
#define CSC_DESTINATION_BASE_MEMORY_EXTERNAL_CS0                    2
#define CSC_DESTINATION_BASE_MEMORY_EXTERNAL_CS1                    3
#define CSC_DESTINATION_BASE_ADDRESS                                25:0

/* CSC Control */
#define CSC_CONTROL                                                 0x1000FC
#define CSC_CONTROL_STATUS                                          31:31
#define CSC_CONTROL_STATUS_STOP                                     0
#define CSC_CONTROL_STATUS_START                                    1
#define CSC_CONTROL_SOURCE_FORMAT                                   30:28
#define CSC_CONTROL_SOURCE_FORMAT_YUV422                            0
#define CSC_CONTROL_SOURCE_FORMAT_YUV420I                           1
#define CSC_CONTROL_SOURCE_FORMAT_YUV420                            2
#define CSC_CONTROL_SOURCE_FORMAT_RGB565                            6
#define CSC_CONTROL_SOURCE_FORMAT_RGBX888                           7
#define CSC_CONTROL_DESTINATION_FORMAT                              27:26
#define CSC_CONTROL_DESTINATION_FORMAT_RGB565                       0
#define CSC_CONTROL_DESTINATION_FORMAT_RGBX888                      1
#define CSC_CONTROL_HORIZONTAL_LINEAR_CONTROL                       25:25
#define CSC_CONTROL_HORIZONTAL_LINEAR_CONTROL_DISABLE               0
#define CSC_CONTROL_HORIZONTAL_LINEAR_CONTROL_ENABLE                1
#define CSC_CONTROL_VERTICAL_LINEAR_CONTROL                         24:24
#define CSC_CONTROL_VERTICAL_LINEAR_CONTROL_DISABLE                 0
#define CSC_CONTROL_VERTICAL_LINEAR_CONTROL_ENABLE                  1
#define CSC_CONTROL_BYTE_ORDER                                      23:23
#define CSC_CONTROL_BYTE_ORDER_YUYV_UVUV                            0
#define CSC_CONTROL_BYTE_ORDER_UYVY_VUVU                            1

/*****************************************************************************\
 *                             2D ENGINE DATA PORT                           *
\*****************************************************************************/

#define DE_DATA_PORT                                                0x110000
#define DE_DATA_PORT_SIZE                                           0x040000

#endif /* _SM501_H_ */

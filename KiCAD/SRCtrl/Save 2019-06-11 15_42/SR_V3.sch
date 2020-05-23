EESchema Schematic File Version 4
LIBS:SR_V3-cache
EELAYER 29 0
EELAYER END
$Descr User 9843 7874
encoding utf-8
Sheet 1 1
Title "Superrollo Controll for GW60 with ESP8266"
Date "2019-06-06"
Rev "2"
Comp "BigPet"
Comment1 "GW 60 controll unit extension board"
Comment2 "- ESP8266: WEMOS D1 mini"
Comment3 "- Hall Sensor for position detection"
Comment4 "- optional 1-wire teperature sensor "
$EndDescr
$Comp
L tlp504:TLP504A U1
U 1 1 5C9A21C0
P 4450 1800
F 0 "U1" H 4450 1600 50  0000 C CNN
F 1 "TLP504A" H 4450 2034 50  0000 C CNN
F 2 "Housings_DIP:DIP-8_W7.62mm_LongPads" H 4250 1600 50  0001 L CIN
F 3 "\\\\192.168.178.21\\NasData\\Projects\\Smart Home\\SRollo\\Doc\\Datasheets\\TLP504A_data.pdf" H 4450 1800 50  0001 L CNN
	1    4450 1800
	1    0    0    -1  
$EndComp
$Comp
L tlp504:TLP504A U1
U 2 1 5C9A2AA2
P 4450 2350
F 0 "U1" H 4450 2150 50  0000 C CNN
F 1 "TLP504A" H 4450 2584 50  0000 C CNN
F 2 "Housings_DIP:DIP-8_W7.62mm_LongPads" H 4250 2150 50  0001 L CIN
F 3 "\\\\192.168.178.21\\NasData\\Projects\\Smart Home\\SRollo\\Doc\\Datasheets\\TLP504A_data.pdf" H 4450 2350 50  0001 L CNN
	2    4450 2350
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5C9A58FC
P 3350 1700
F 0 "R1" V 3250 1700 50  0000 C CNN
F 1 "2k2" V 3350 1700 50  0000 C CNN
F 2 "Resistors_ThroughHole:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 3280 1700 50  0001 C CNN
F 3 "~" H 3350 1700 50  0001 C CNN
	1    3350 1700
	0    1    1    0   
$EndComp
$Comp
L Device:R R2
U 1 1 5C9A5E30
P 3350 2250
F 0 "R2" V 3250 2250 50  0000 C CNN
F 1 "2k2" V 3350 2250 50  0000 C CNN
F 2 "Resistors_ThroughHole:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 3280 2250 50  0001 C CNN
F 3 "~" H 3350 2250 50  0001 C CNN
	1    3350 2250
	0    1    1    0   
$EndComp
Wire Wire Line
	4150 1900 4000 1900
Wire Wire Line
	4150 2450 4000 2450
$Comp
L power:GND #PWR04
U 1 1 5C9A7C91
P 4900 2600
F 0 "#PWR04" H 4900 2350 50  0001 C CNN
F 1 "GND" H 4905 2427 50  0000 C CNN
F 2 "" H 4900 2600 50  0001 C CNN
F 3 "" H 4900 2600 50  0001 C CNN
	1    4900 2600
	1    0    0    -1  
$EndComp
$Comp
L Diode:BAT41 D1
U 1 1 5C99D8AD
P 3800 1900
F 0 "D1" H 3800 2000 50  0000 C CNN
F 1 "BAT41" H 3800 1800 50  0000 C CNN
F 2 "Diodes_ThroughHole:D_DO-35_SOD27_P7.62mm_Horizontal" H 3800 1725 50  0001 C CNN
F 3 "http://www.vishay.com/docs/85659/bat41.pdf" H 3800 1900 50  0001 C CNN
	1    3800 1900
	1    0    0    -1  
$EndComp
$Comp
L Diode:BAT41 D2
U 1 1 5C99EBB6
P 3800 2450
F 0 "D2" H 3800 2550 50  0000 C CNN
F 1 "BAT41" H 3800 2350 50  0000 C CNN
F 2 "Diodes_ThroughHole:D_DO-35_SOD27_P7.62mm_Horizontal" H 3800 2275 50  0001 C CNN
F 3 "http://www.vishay.com/docs/85659/bat41.pdf" H 3800 2450 50  0001 C CNN
	1    3800 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2850 1700 3200 1700
Wire Wire Line
	2850 2250 3200 2250
$Comp
L tlp504:TLP504A U3
U 1 1 5C9A528C
P 4450 3150
F 0 "U3" H 4450 3350 50  0000 C CNN
F 1 "TLP504A" H 4450 2924 50  0000 C CNN
F 2 "Housings_DIP:DIP-8_W7.62mm_LongPads" H 4250 2950 50  0001 L CIN
F 3 "\\\\192.168.178.21\\NasData\\Projects\\Smart Home\\SRollo\\Doc\\Datasheets\\TLP504A_data.pdf" H 4450 3150 50  0001 L CNN
	1    4450 3150
	-1   0    0    1   
$EndComp
$Comp
L tlp504:TLP504A U3
U 2 1 5C9A64CC
P 4450 3700
F 0 "U3" H 4450 3900 50  0000 C CNN
F 1 "TLP504A" H 4450 3474 50  0000 C CNN
F 2 "Housings_DIP:DIP-8_W7.62mm_LongPads" H 4250 3500 50  0001 L CIN
F 3 "\\\\192.168.178.21\\NasData\\Projects\\Smart Home\\SRollo\\Doc\\Datasheets\\TLP504A_data.pdf" H 4450 3700 50  0001 L CNN
	2    4450 3700
	-1   0    0    1   
$EndComp
Wire Wire Line
	4150 3050 4000 3050
Wire Wire Line
	4000 3050 4000 3550
Wire Wire Line
	4000 3600 4150 3600
$Comp
L Device:R R3
U 1 1 5C9AEDEB
P 5400 3050
F 0 "R3" V 5300 3050 50  0000 C CNN
F 1 "560" V 5400 3050 50  0000 C CNN
F 2 "Resistors_ThroughHole:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5330 3050 50  0001 C CNN
F 3 "~" H 5400 3050 50  0001 C CNN
	1    5400 3050
	0    1    1    0   
$EndComp
$Comp
L Device:R R4
U 1 1 5C9AF440
P 5400 3600
F 0 "R4" V 5300 3600 50  0000 C CNN
F 1 "560" V 5400 3600 50  0000 C CNN
F 2 "Resistors_ThroughHole:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5330 3600 50  0001 C CNN
F 3 "~" H 5400 3600 50  0001 C CNN
	1    5400 3600
	0    1    1    0   
$EndComp
Wire Wire Line
	4750 3250 4900 3250
Wire Wire Line
	4900 3250 4900 3800
Wire Wire Line
	4750 3800 4900 3800
Connection ~ 4900 3800
Wire Wire Line
	4900 3800 4900 3900
Wire Wire Line
	2850 3250 4150 3250
Wire Wire Line
	2850 3800 4150 3800
Wire Wire Line
	4750 3050 5250 3050
Wire Wire Line
	5550 3050 6250 3050
Wire Wire Line
	4750 3600 5250 3600
Wire Wire Line
	5550 3600 6250 3600
Connection ~ 4000 3550
Wire Wire Line
	4000 3550 4000 3600
Wire Wire Line
	4750 1700 4900 1700
Wire Wire Line
	4900 1700 4900 2250
Wire Wire Line
	4900 2250 4750 2250
Wire Wire Line
	4900 2250 4900 2600
Connection ~ 4900 2250
Wire Wire Line
	3500 2250 3650 2250
Wire Wire Line
	3500 1700 3650 1700
Wire Wire Line
	4000 1900 4000 2450
Wire Wire Line
	3950 2450 4000 2450
Connection ~ 4000 2450
Wire Wire Line
	3950 1900 4000 1900
Connection ~ 4000 1900
Wire Wire Line
	3650 1900 3650 1700
Connection ~ 3650 1700
Wire Wire Line
	3650 1700 4150 1700
Wire Wire Line
	3650 2450 3650 2250
Connection ~ 3650 2250
Wire Wire Line
	3650 2250 4150 2250
Wire Wire Line
	4750 2450 6250 2450
Wire Wire Line
	4750 1900 6250 1900
Wire Wire Line
	2850 3550 4000 3550
$Comp
L power:VCC #PWR010
U 1 1 5C9F8925
P 2250 4000
F 0 "#PWR010" H 2250 3850 50  0001 C CNN
F 1 "VCC" H 2267 4173 50  0000 C CNN
F 2 "" H 2250 4000 50  0001 C CNN
F 3 "" H 2250 4000 50  0001 C CNN
	1    2250 4000
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR09
U 1 1 5C9B292B
P 4900 3900
F 0 "#PWR09" H 4900 3750 50  0001 C CNN
F 1 "VCC" H 4918 4073 50  0000 C CNN
F 2 "" H 4900 3900 50  0001 C CNN
F 3 "" H 4900 3900 50  0001 C CNN
	1    4900 3900
	-1   0    0    1   
$EndComp
Text Label 2850 1700 0    50   ~ 0
MotorIn1
Text Label 2850 2250 0    50   ~ 0
MotorIn2
Text Label 2850 3250 0    50   ~ 0
TasterOut1
Text Label 2850 3800 0    50   ~ 0
TasterOut2
Text Label 2850 3550 0    50   ~ 0
TasterOutCom
Text Label 2400 1750 2    50   ~ 0
MotorIn2
Text Label 2400 1850 2    50   ~ 0
MotorIn1
Text Label 2400 1950 2    50   ~ 0
TasterOut1
Text Label 2400 2050 2    50   ~ 0
TasterOutCom
Text Label 2400 2150 2    50   ~ 0
TasterOut2
Text Label 6250 3600 2    50   ~ 0
Taster2
Text Label 6250 3050 2    50   ~ 0
Taster1
Text Label 6250 2450 2    50   ~ 0
Motor2
Text Label 6250 1900 2    50   ~ 0
Motor1
Text Label 2250 4100 2    50   ~ 0
DQ
$Comp
L power:GND #PWR011
U 1 1 5CA3089C
P 2250 4200
F 0 "#PWR011" H 2250 3950 50  0001 C CNN
F 1 "GND" H 2255 4027 50  0000 C CNN
F 2 "" H 2250 4200 50  0001 C CNN
F 3 "" H 2250 4200 50  0001 C CNN
	1    2250 4200
	1    0    0    -1  
$EndComp
Text Label 2400 2250 2    50   ~ 0
HallSensorIn
$Comp
L power:GND #PWR03
U 1 1 5CA38E59
P 2400 2350
F 0 "#PWR03" H 2400 2100 50  0001 C CNN
F 1 "GND" H 2405 2177 50  0000 C CNN
F 2 "" H 2400 2350 50  0001 C CNN
F 3 "" H 2400 2350 50  0001 C CNN
	1    2400 2350
	1    0    0    -1  
$EndComp
Text Label 8200 2450 2    50   ~ 0
Taster1
Text Label 8200 2550 2    50   ~ 0
Taster2
Text Label 8200 2650 2    50   ~ 0
HallSensorIn
Text Label 8200 2250 2    50   ~ 0
DQ
NoConn ~ 7050 1550
$Comp
L power:VCC #PWR02
U 1 1 5CA2D745
P 7250 1550
F 0 "#PWR02" H 7250 1400 50  0001 C CNN
F 1 "VCC" H 7267 1723 50  0000 C CNN
F 2 "" H 7250 1550 50  0001 C CNN
F 3 "" H 7250 1550 50  0001 C CNN
	1    7250 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	7250 1600 7250 1550
NoConn ~ 7550 2750
Wire Wire Line
	7550 2650 8200 2650
Wire Wire Line
	7550 2550 8200 2550
Wire Wire Line
	7550 2450 8200 2450
NoConn ~ 7550 1950
NoConn ~ 7550 1850
NoConn ~ 6750 2350
NoConn ~ 6750 2250
Wire Wire Line
	7550 2050 8200 2050
Wire Wire Line
	7550 2150 8200 2150
Wire Wire Line
	7550 2250 8200 2250
NoConn ~ 7550 2350
$Comp
L power:GND #PWR06
U 1 1 5C9FD1FF
P 7150 3150
F 0 "#PWR06" H 7150 2900 50  0001 C CNN
F 1 "GND" H 7155 2977 50  0000 C CNN
F 2 "" H 7150 3150 50  0001 C CNN
F 3 "" H 7150 3150 50  0001 C CNN
	1    7150 3150
	1    0    0    -1  
$EndComp
NoConn ~ 6750 1950
$Comp
L MCU_Module:WeMos_D1_mini U2
U 1 1 5C9AA4E6
P 7150 2350
F 0 "U2" H 7300 1600 50  0000 L TNN
F 1 "WeMos_D1_mini" V 6750 1650 50  0000 L CNN
F 2 "Module:WEMOS_D1_mini_light" H 7150 1200 50  0001 C CNN
F 3 "https://wiki.wemos.cc/products:d1:d1_mini#documentation" H 5300 1200 50  0001 C CNN
	1    7150 2350
	1    0    0    -1  
$EndComp
Connection ~ 7250 1550
$Comp
L Connector:Conn_01x08_Female J1
U 1 1 5CF996DA
P 1450 1950
F 0 "J1" H 1400 1450 50  0000 C CNN
F 1 "from GW60" H 1250 2400 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x08_Pitch2.54mm" H 1450 1950 50  0001 C CNN
F 3 "~" H 1450 1950 50  0001 C CNN
	1    1450 1950
	-1   0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x04_Female J2
U 1 1 5CF9A639
P 1450 3050
F 0 "J2" H 1400 2750 50  0000 C CNN
F 1 "Step-Down 24V to 3.3V" H 1200 3300 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x04_Pitch2.54mm" H 1450 3050 50  0001 C CNN
F 3 "~" H 1450 3050 50  0001 C CNN
	1    1450 3050
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1650 1750 2400 1750
Wire Wire Line
	1650 1850 2400 1850
Wire Wire Line
	1650 1950 2400 1950
Wire Wire Line
	1650 2050 2400 2050
Wire Wire Line
	1650 2150 2400 2150
Wire Wire Line
	1650 2250 2400 2250
Wire Wire Line
	1650 2350 2400 2350
Text Label 2400 2350 2    50   ~ 0
GW60_GND
Text Label 2350 1650 2    50   ~ 0
GW60_Supply
Wire Wire Line
	1650 1650 2350 1650
$Comp
L power:+24V #PWR01
U 1 1 5CA309AC
P 2350 1550
F 0 "#PWR01" H 2350 1400 50  0001 C CNN
F 1 "+24V" H 2365 1723 50  0000 C CNN
F 2 "" H 2350 1550 50  0001 C CNN
F 3 "" H 2350 1550 50  0001 C CNN
	1    2350 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2350 1650 2350 1550
Wire Wire Line
	1650 2950 2050 2950
Text Label 2050 2950 2    50   ~ 0
EN
Text Label 2050 3050 2    50   ~ 0
IN+
Text Label 2050 3150 2    50   ~ 0
GND
Text Label 2050 3250 2    50   ~ 0
V0+
$Comp
L power:+24V #PWR05
U 1 1 5CFE6A18
P 2250 2800
F 0 "#PWR05" H 2250 2650 50  0001 C CNN
F 1 "+24V" H 2265 2973 50  0000 C CNN
F 2 "" H 2250 2800 50  0001 C CNN
F 3 "" H 2250 2800 50  0001 C CNN
	1    2250 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	1650 3050 2250 3050
$Comp
L power:GND #PWR08
U 1 1 5CFFAC8A
P 2450 3400
F 0 "#PWR08" H 2450 3150 50  0001 C CNN
F 1 "GND" H 2455 3227 50  0000 C CNN
F 2 "" H 2450 3400 50  0001 C CNN
F 3 "" H 2450 3400 50  0001 C CNN
	1    2450 3400
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR07
U 1 1 5CFFE040
P 2250 3400
F 0 "#PWR07" H 2250 3250 50  0001 C CNN
F 1 "VCC" H 2267 3573 50  0000 C CNN
F 2 "" H 2250 3400 50  0001 C CNN
F 3 "" H 2250 3400 50  0001 C CNN
	1    2250 3400
	-1   0    0    1   
$EndComp
Wire Wire Line
	2250 3050 2250 2800
Wire Wire Line
	2450 3150 2450 3400
Wire Wire Line
	1650 3150 2450 3150
Wire Wire Line
	2250 3250 2250 3400
Wire Wire Line
	1650 3250 2250 3250
NoConn ~ 2050 2950
Text Notes 1650 1650 0    50   ~ 0
(rt)
Text Notes 1650 1750 0    50   ~ 0
(bl)
Text Notes 1650 1850 0    50   ~ 0
(bn)
Text Notes 1650 1950 0    50   ~ 0
(gn)
Text Notes 1650 2050 0    50   ~ 0
(gr)
Text Notes 1650 2150 0    50   ~ 0
(vio)
Text Notes 1650 2250 0    50   ~ 0
(ge)
Text Notes 1650 2350 0    50   ~ 0
(sw)
Wire Wire Line
	1650 4100 2250 4100
Wire Wire Line
	1650 4200 2250 4200
Text Label 8200 2150 2    50   ~ 0
Motor2
Text Label 8200 2050 2    50   ~ 0
Motor1
Wire Wire Line
	1650 4000 2250 4000
$Comp
L Connector:Conn_01x03_Female J3
U 1 1 5CFEDC44
P 1450 4100
F 0 "J3" H 1400 3900 50  0000 C CNN
F 1 "DS18x20" H 1250 4400 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x03_Pitch2.54mm" H 1450 4100 50  0001 C CNN
F 3 "~" H 1450 4100 50  0001 C CNN
	1    1450 4100
	-1   0    0    1   
$EndComp
$EndSCHEMATC

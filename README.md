# PSOC&trade; Edge MCU: Bluetooth&reg; LE sensor hub

This code example features a Bluetooth&reg; LE custom GATT Server (PSOC&trade; Edge E84 MCU with AIROC&trade; CYW55513 Wi-Fi & Bluetooth&reg; combo chip) and the GATT client (smartphone). The PSOC&trade; Edge E84 MCU is interfaced with a BMI270 Inertial Measurement Unit (IMU) sensor over an I2C interface using the BMI270 motion sensor library. The motion and temperature sensor readings are sent to the GATT Client over GATT notification in particular intervals. This interval can be changed by pressing the USER BTN 1 on the kit and the notify interval is stored in the non-volatile memory (NVM) using the [kv-store](https://github.com/Infineon/kv-store) library. 

This code example has a three project structure: CM33 secure, CM33 non-secure, and CM55 projects. All three projects are programmed to the external QSPI flash and executed in Execute in Place (XIP) mode. Extended boot launches the CM33 secure project from a fixed location in the external flash, which then configures the protection settings and launches the CM33 non-secure application. Additionally, CM33 non-secure application enables CM55 CPU and launches the CM55 application.

[View this README on GitHub.](https://github.com/Infineon/mtb-example-psoc-edge-btstack-sensor-hub)

[Provide feedback on this code example.](https://cypress.co1.qualtrics.com/jfe/form/SV_1NTns53sK2yiljn?Q_EED=eyJVbmlxdWUgRG9jIElkIjoiQ0UyMzk2NDciLCJTcGVjIE51bWJlciI6IjAwMi0zOTY0NyIsIkRvYyBUaXRsZSI6IlBTT0MmdHJhZGU7IEVkZ2UgTUNVOiBCbHVldG9vdGgmcmVnOyBMRSBzZW5zb3IgaHViIiwicmlkIjoic3VyZXNoa3VtYXJhIiwiRG9jIHZlcnNpb24iOiIyLjAuMCIsIkRvYyBMYW5ndWFnZSI6IkVuZ2xpc2giLCJEb2MgRGl2aXNpb24iOiJNQ0QiLCJEb2MgQlUiOiJJQ1ciLCJEb2MgRmFtaWx5IjoiUFNPQyJ9)

See the [Design and implementation](docs/design_and_implementation.md) for the functional description of this code example.


## Requirements

- [ModusToolbox&trade;](https://www.infineon.com/modustoolbox) v3.6 or later (tested with v3.6)
- Board support package (BSP) minimum required version: 1.0.0
- Programming language: C
- Associated parts: All [PSOC&trade; Edge MCU](https://www.infineon.com/products/microcontroller/32-bit-psoc-arm-cortex/32-bit-psoc-edge-arm) parts


## Supported toolchains (make variable 'TOOLCHAIN')

- GNU Arm&reg; Embedded Compiler v14.2.1 (`GCC_ARM`) – Default value of `TOOLCHAIN`
- Arm&reg; Compiler v6.22 (`ARM`)
- IAR C/C++ Compiler v9.50.2 (`IAR`)
- LLVM Embedded Toolchain for Arm&reg; v19.1.5 (`LLVM_ARM`)


## Supported kits (make variable 'TARGET')

- [PSOC&trade; Edge E84 Evaluation Kit](https://www.infineon.com/KIT_PSE84_EVAL) (`KIT_PSE84_EVAL_EPC2`) – Default value of `TARGET`
- [PSOC&trade; Edge E84 Evaluation Kit](https://www.infineon.com/KIT_PSE84_EVAL) (`KIT_PSE84_EVAL_EPC4`)


## Hardware setup

This example uses the board's default configuration. See the kit user guide to ensure that the board is configured correctly.

Ensure the following jumper and pin configuration on board.
- BOOT SW must be in the HIGH/ON position
- J20 and J21 must be in the tristate/not connected (NC) position


## Software setup

See the [ModusToolbox&trade; tools package installation guide](https://www.infineon.com/ModusToolboxInstallguide) for information about installing and configuring the tools package.

Install a terminal emulator if you do not have one. Instructions in this document use [Tera Term](https://teratermproject.github.io/index-en.html).

Download and install the AIROC&trade; Bluetooth&reg; Connect App on your [iOS](https://apps.apple.com/in/app/airoc-bluetooth-connect-app/id6443702288) or [Android](https://play.google.com/store/apps/details?id=com.infineon.airocbluetoothconnect) phone.

Scan the following QR codes from your mobile phone to download the AIROC&trade; Bluetooth&reg; Connect App.

![](./images/qr.png)

This example requires no additional software or tools.


## Operation

See [Using the code example](docs/using_the_code_example.md) for instructions on creating a project, opening it in various supported IDEs, and performing tasks, such as building, programming, and debugging the application within the respective IDEs.

1. Connect the board to your PC using the provided USB cable through the KitProg3 USB connector

2. Open a terminal program and select the KitProg3 COM port. Set the serial port parameters to 8N1 and 115200 baud

3. After programming, the application starts automatically. Observe the messages on the UART terminal and wait for the device to make all the required Bluetooth&reg; LE initializations. Use the KitProg3 COM port to view the Bluetooth&reg; stack and application trace messages in the terminal window as shown in **Figure 1**

4. The USER LED3 starts blinking to indicate that advertisement has started and stops blinking after few blinks as the device enters deepsleep mode.

   **Figure 1. Terminal log messages on KitProg3 COM port when notify interval is not set**

   ![](images/terminal-log-messages1.png)

   **Figure 2. Terminal log messages on KitProg3 COM port when notify interval is set**

   ![](images/terminal-log-messages2.png)

   > **Note:** The message "Notify interval not stored in NVM!" (see **Figure 1**) will be displayed on the terminal, when the application is first time programmed where the notify interval is not available in NVM or the notify interval is cleared by using kv-store reset. If this message displays after storing notify interval, it indicates a failure in reading the non-volatile memory (NVM)


### Test using AIROC&trade; Bluetooth&reg; Connect mobile app

1. Turn ON Bluetooth&reg; and Location on your Android or iOS device

2. Launch the AIROC&trade; Bluetooth&reg; Connect App

3. Swipe down on the AIROC&trade; Bluetooth&reg; Connect App home screen to start scanning for Bluetooth&reg; LE peripheral devices. Your device named “Sensor Hub” displays in the AIROC&trade; Bluetooth&reg; Connect App home screen. Select your device to establish a Bluetooth&reg; LE connection. The USER LED3 stays ON indicating that the device has been connected

4. Select **GATT DB** and select the **Service** available

      **Figure 3. Select Sensor hub and GATT DB**

      ![](images/bleconnectapp_1.png)


#### Motion sensor data notification

1. Select the first characteristic **0005EA05-0000-1000-8000-00805F9B0131** for motion sensor data (accelerometer, gyroscope, and orientation) notification

2. Select **Notify**
   
3. The device will start sending GATT notifications to the mobile in hexadecimal format in the **HEX** section in interval of 100 ms by default. Additionally, you will see the motion sensor data in the UART terminal <br>
   The USER LED1 will turn ON to indicate that notification has started

   **Figure 4. Steps to get motion sensor data notifications using AIROC&trade; Bluetooth&reg; Connect App on Android**

   ![](images/motionsensor_1.png)

   **Figure 4** shows how to interpret the Hex values of motion sensor notification data

   **Figure 5. Nomenclature of the sensor data Hex value**

   ![](images/motionsensor_2.png)

   Note that in **Figure 4**, for all the values, the first 2 digits are the least significant bits (LSbs) and the last 2 digits are the most significant bits (MSb) <br>

   For example, the Acc.x value - d101 (d1-MSB, 01-LSB) in **Figure 5** is to be considered as 01d1 (01-MSB, d1-LSB) and the equivalent decimal value is 465 <br>

   Similarly, the orientation value - 0500 (05-MSB, 00-LSB) in **Figure 5** is to be considered as 0005 (00-MSB, 05-LSB) and the equal decimal value is 5 and the corresponding definition is ORIENTATION_DISP_UP 

   Following are the corresponding definitions for orientation data:
   - ORIENTATION_NULL            = 0   
   - ORIENTATION_TOP_EDGE        = 1    
   - ORIENTATION_BOTTOM_EDGE     = 2   
   - ORIENTATION_LEFT_EDGE       = 3   
   - ORIENTATION_RIGHT_EDGE      = 4   
   - ORIENTATION_DISP_UP         = 5  
   - ORIENTATION_DISP_DOWN       = 6 

   **Figure 6. UART terminal log of motion sensor data**

   ![](images/motion_sensor3.png)

4. Select **Stop Notify** in AIROC&trade; Bluetooth&reg; Connect App to stop the motion sensor data notification. The USER LED1 will turn OFF to indicate that the notification has stopped


#### Temperature sensor data notification

1. Select the second characteristic **0005EA07-0000-1000-8000-00805F9B0131** for temperature sensor data notification in degree Celsius

2. Select **Notify**
   
3. The device will start sending GATT notifications to the mobile in hexadecimal values in the **hex** section in interval of 100 ms by default. Additionally, you will see the temperature sensor data in the UART terminal. The USER LED1 will turn ON to indicate that notification has started

   **Figure 7. Steps to get temperature sensor data notifications using AIROC&trade; Bluetooth&reg; Connect App on Android**

   ![](images/tempsensor_1.png)

   **Figure 8. UART terminal log of temperature sensor data**

   ![](images/temp_sensor2.png)

4. Select **Stop Notify** in AIROC&trade; Bluetooth&reg; Connect App to stop the temperature sensor data notification. The USER LED1 will turn OFF to indicate that the notification has stopped

5. Click the **USER BTN1 (SW2)** any time to change the notification interval. This notify interval is stored in the NVM using the kv-store library. **Figure 9** shows the terminal logs for different notification intervals

   **Figure 9. Terminal log for notification intervals**

   ![](images/terminal_notify.png)

6. Long press the **USER BTN1 (SW2)** for 3 seconds to reset the kv-store library which resets the NVM. Note that after resetting the NVM, reset the kit to receive notification in default (100 ms) interval


### Resources and settings

This section explains the ModusToolbox&trade; software resources and their configurations as used in this code example. Note that all the configuration explained in this section has already been implemented in the code example.

- **Bluetooth&reg; Configurator:** The Bluetooth&reg; peripheral has a configurator called the “Bluetooth&reg; Configurator” that is used to generate the Bluetooth&reg; LE GATT database and various Bluetooth&reg; settings for the application. These settings are stored in the file named *design.cybt*.

See the [Bluetooth&reg; Configurator guide](https://www.infineon.com/ModusToolboxBLEConfig) for more details.


## Related resources

Resources  | Links
-----------|----------------------------------
Application notes  | [AN235935](https://www.infineon.com/AN235935) – Getting started with PSOC&trade; Edge E8 MCU on ModusToolbox&trade; software <br> [AN236697](https://www.infineon.com/AN236697) – Getting started with PSOC&trade; MCU and AIROC&trade; Connectivity devices <br>
Code examples  | [Using ModusToolbox&trade;](https://github.com/Infineon/Code-Examples-for-ModusToolbox-Software) on GitHub
Device documentation | [PSOC&trade; Edge MCU datasheets](https://www.infineon.com/products/microcontroller/32-bit-psoc-arm-cortex/32-bit-psoc-edge-arm#documents) <br> [PSOC&trade; Edge MCU reference manuals](https://www.infineon.com/products/microcontroller/32-bit-psoc-arm-cortex/32-bit-psoc-edge-arm#documents)
Development kits | Select your kits from the [Evaluation board finder](https://www.infineon.com/cms/en/design-support/finder-selection-tools/product-finder/evaluation-board)
Libraries  | [mtb-dsl-pse8xxgp](https://github.com/Infineon/mtb-dsl-pse8xxgp) – Device support library for PSE8XXGP <br> [retarget-io](https://github.com/Infineon/retarget-io) – Utility library to retarget STDIO messages to a UART port <br> [btstack-integration](https://github.com/Infineon/btstack-integration) – The btstack-integration hosts platform adaptation layer (porting layer) between AIROC&trade; BTSTACK and Infineon's different hardware platforms <br> [kv-store](https://github.com/Infineon/kv-store) – This library provides a convenient way to store information as key-value pairs in non-volatile storage
Tools  | [ModusToolbox&trade;](https://www.infineon.com/modustoolbox) – ModusToolbox&trade; software is a collection of easy-to-use libraries and tools enabling rapid development with Infineon MCUs for applications ranging from wireless and cloud-connected systems, edge AI/ML, embedded sense and control, to wired USB connectivity using PSOC&trade; Industrial/IoT MCUs, AIROC&trade; Wi-Fi and Bluetooth&reg; connectivity devices, XMC&trade; Industrial MCUs, and EZ-USB&trade;/EZ-PD&trade; wired connectivity controllers. ModusToolbox&trade; incorporates a comprehensive set of BSPs, HAL, libraries, configuration tools, and provides support for industry-standard IDEs to fast-track your embedded application development

<br>


## Other resources

Infineon provides a wealth of data at [www.infineon.com](https://www.infineon.com) to help you select the right device, and quickly and effectively integrate it into your design.


## Document history

Document title: *CE239647* – *PSOC&trade; Edge MCU: Bluetooth&reg; LE sensor hub*

 Version | Description of change
 ------- | ---------------------
 1.x.0   | New code example <br> Early access release
 2.0.0   | GitHub release
<br>


All referenced product or service names and trademarks are the property of their respective owners.

The Bluetooth&reg; word mark and logos are registered trademarks owned by Bluetooth SIG, Inc., and any use of such marks by Infineon is under license.

PSOC&trade;, formerly known as PSoC&trade;, is a trademark of Infineon Technologies. Any references to PSoC&trade; in this document or others shall be deemed to refer to PSOC&trade;.

---------------------------------------------------------

© Cypress Semiconductor Corporation, 2023-2025. This document is the property of Cypress Semiconductor Corporation, an Infineon Technologies company, and its affiliates ("Cypress").  This document, including any software or firmware included or referenced in this document ("Software"), is owned by Cypress under the intellectual property laws and treaties of the United States and other countries worldwide.  Cypress reserves all rights under such laws and treaties and does not, except as specifically stated in this paragraph, grant any license under its patents, copyrights, trademarks, or other intellectual property rights.  If the Software is not accompanied by a license agreement and you do not otherwise have a written agreement with Cypress governing the use of the Software, then Cypress hereby grants you a personal, non-exclusive, nontransferable license (without the right to sublicense) (1) under its copyright rights in the Software (a) for Software provided in source code form, to modify and reproduce the Software solely for use with Cypress hardware products, only internally within your organization, and (b) to distribute the Software in binary code form externally to end users (either directly or indirectly through resellers and distributors), solely for use on Cypress hardware product units, and (2) under those claims of Cypress's patents that are infringed by the Software (as provided by Cypress, unmodified) to make, use, distribute, and import the Software solely for use with Cypress hardware products.  Any other use, reproduction, modification, translation, or compilation of the Software is prohibited.
<br>
TO THE EXTENT PERMITTED BY APPLICABLE LAW, CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH REGARD TO THIS DOCUMENT OR ANY SOFTWARE OR ACCOMPANYING HARDWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  No computing device can be absolutely secure.  Therefore, despite security measures implemented in Cypress hardware or software products, Cypress shall have no liability arising out of any security breach, such as unauthorized access to or use of a Cypress product. CYPRESS DOES NOT REPRESENT, WARRANT, OR GUARANTEE THAT CYPRESS PRODUCTS, OR SYSTEMS CREATED USING CYPRESS PRODUCTS, WILL BE FREE FROM CORRUPTION, ATTACK, VIRUSES, INTERFERENCE, HACKING, DATA LOSS OR THEFT, OR OTHER SECURITY INTRUSION (collectively, "Security Breach").  Cypress disclaims any liability relating to any Security Breach, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any Security Breach.  In addition, the products described in these materials may contain design defects or errors known as errata which may cause the product to deviate from published specifications. To the extent permitted by applicable law, Cypress reserves the right to make changes to this document without further notice. Cypress does not assume any liability arising out of the application or use of any product or circuit described in this document. Any information provided in this document, including any sample design information or programming code, is provided only for reference purposes.  It is the responsibility of the user of this document to properly design, program, and test the functionality and safety of any application made of this information and any resulting product.  "High-Risk Device" means any device or system whose failure could cause personal injury, death, or property damage.  Examples of High-Risk Devices are weapons, nuclear installations, surgical implants, and other medical devices.  "Critical Component" means any component of a High-Risk Device whose failure to perform can be reasonably expected to cause, directly or indirectly, the failure of the High-Risk Device, or to affect its safety or effectiveness.  Cypress is not liable, in whole or in part, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any use of a Cypress product as a Critical Component in a High-Risk Device. You shall indemnify and hold Cypress, including its affiliates, and its directors, officers, employees, agents, distributors, and assigns harmless from and against all claims, costs, damages, and expenses, arising out of any claim, including claims for product liability, personal injury or death, or property damage arising from any use of a Cypress product as a Critical Component in a High-Risk Device. Cypress products are not intended or authorized for use as a Critical Component in any High-Risk Device except to the limited extent that (i) Cypress's published data sheet for the product explicitly states Cypress has qualified the product for use in a specific High-Risk Device, or (ii) Cypress has given you advance written authorization to use the product as a Critical Component in the specific High-Risk Device and you have signed a separate indemnification agreement.
<br>
Cypress, the Cypress logo, and combinations thereof, ModusToolbox, PSoC, CAPSENSE, EZ-USB, F-RAM, and TRAVEO are trademarks or registered trademarks of Cypress or a subsidiary of Cypress in the United States or in other countries. For a more complete list of Cypress trademarks, visit www.infineon.com. Other names and brands may be claimed as property of their respective owners.

// The start of this code came from 
// https://forums.raspberrypi.com/viewtopic.php?t=353889&sid=2b10dcb0eca390f135ee8c64da253a9c

// After getting quite a bit of understanding I also found this useful
// https://github.com/makercrew/dbus-sample

#include <cstdlib>
#include <cstring>
#include <dbus/dbus.h> //  sudo apt install libdbus-1-dev
#include <iomanip>
#include <iostream>
#include <iterator>
#include <locale>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
// Do this while running ble-dbus to see what else could be picked up from an advertisement message
// gdbus introspect --system --dest org.bluez --object-path /org/bluez --recurse

// I'm going to use this to create a map of the bluetooth addresses and names of the Govee Devices 
/* BD Address */
typedef struct {
    uint8_t b[6];
} __attribute__((packed)) bdaddr_t;
std::map<bdaddr_t, std::string> GoveeBluetoothTitles;
// The old method on adding the Govee devices is to recognize manufacturer data of a specific format and decode it
// I've got to figure out how to find the same data in the data returned from DBus
/*
bool Govee_Temp::ReadMSG(const uint8_t * const data)
{
    bool rval = false;
    const size_t data_len = data[0];
    if (data[1] == 0xFF) // https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile/ «Manufacturer Specific Data»
    {
        if ((data_len == 9) && (data[2] == 0x88) && (data[3] == 0xEC)) // GVH5075_xxxx
        {
            if (Model == ThermometerType::Unknown)
                Model = ThermometerType::H5075;
            // This data came from https://github.com/Thrilleratplay/GoveeWatcher
            // 88ec00 03519e 64 00 Temp: 21.7502°C Temp: 71.1504°F Humidity: 50.2%
            // 2 3 4  5 6 7  8
            int iTemp = int(data[5]) << 16 | int(data[6]) << 8 | int(data[7]);
            bool bNegative = iTemp & 0x800000;	// check sign bit
            iTemp = iTemp & 0x7ffff;			// mask off sign bit
            Temperature[0] = float(iTemp / 1000) / 10.0; // issue #49 fix.
            // After converting the hexadecimal number into decimal the first three digits are the
            // temperature and the last three digits are the humidity.So "03519e" converts to "217502"
            // which means 21.7 °C and 50.2 % humidity without any rounding.
            if (bNegative)						// apply sign bit
                Temperature[0] = -1.0 * Temperature[0];
            Humidity = float(iTemp % 1000) / 10.0;
            Battery = int(data[8]);
            Averages = 1;
            time(&Time);
            TemperatureMin[0] = TemperatureMax[0] = Temperature[0];	//HACK: make sure that these values are set
            rval = true;
        }
        else if ((data_len == 10) && (data[2] == 0x88) && (data[3] == 0xEC))// Govee_H5074_xxxx
        {
            if (Model == ThermometerType::Unknown)
                Model = ThermometerType::H5074;
            // This data came from https://github.com/neilsheps/GoveeTemperatureAndHumidity
            // 88EC00 0902 CD15 64 02 (Temp) 41.378°F (Humidity) 55.81% (Battery) 100%
            // 2 3 4  5 6  7 8  9
            short iTemp = short(data[6]) << 8 | short(data[5]);
            int iHumidity = int(data[8]) << 8 | int(data[7]);
            Temperature[0] = float(iTemp) / 100.0;
            Humidity = float(iHumidity) / 100.0;
            Battery = int(data[9]);
            Averages = 1;
            time(&Time);
            TemperatureMin[0] = TemperatureMax[0] = Temperature[0];	//HACK: make sure that these values are set
            rval = true;
        }
        else if ((data_len == 12) && (data[2] == 0x01) && (data[3] == 0x88) && (data[4] == 0xEC)) // Govee_H5179
        {
            if (Model == ThermometerType::Unknown)
                Model = ThermometerType::H5179;
            // This is from data provided in https://github.com/wcbonner/GoveeBTTempLogger/issues/36
            // 0188EC00 0101 0A0A B018 64 (Temp) 25.7°C (Humidity) 63.2% (Battery) 100% (GVH5179)
            // 2 3 4 5  6 7  8 9  1011 12
            short iTemp = short(data[9]) << 8 | short(data[8]);
            int iHumidity = int(data[11]) << 8 | int(data[10]);
            Temperature[0] = float(iTemp) / 100.0;
            Humidity = float(iHumidity) / 100.0;
            Battery = int(data[12]);
            Averages = 1;
            time(&Time);
            TemperatureMin[0] = TemperatureMax[0] = Temperature[0];	//HACK: make sure that these values are set
            rval = true;
        }
        else if ((data_len == 9) && (data[2] == 0x01) && (data[3] == 0x00)) // GVH5177_xxxx or GVH5174_xxxx or GVH5100_xxxx
        {
            // This is a guess based on the H5075 3 byte encoding
            // 01000101 029D1B 64 (Temp) 62.8324°F (Humidity) 29.1% (Battery) 100%
            // 2 3 4 5  6 7 8  9
            // It appears that the H5174 uses the exact same data format as the H5177, with the difference being the broadcase name starting with GVH5174_
            int iTemp = int(data[6]) << 16 | int(data[7]) << 8 | int(data[8]);
            bool bNegative = iTemp & 0x800000;	// check sign bit
            iTemp = iTemp & 0x7ffff;			// mask off sign bit
            Temperature[0] = float(iTemp) / 10000.0;
            Humidity = float(iTemp % 1000) / 10.0;
            if (bNegative)						// apply sign bit
                Temperature[0] = -1.0 * Temperature[0];
            Battery = int(data[9]);
            Averages = 1;
            time(&Time);
            TemperatureMin[0] = TemperatureMax[0] = Temperature[0];	//HACK: make sure that these values are set
            rval = true;
        }
        else if (data_len == 17 && (data[5] == 0x01) && (data[6] == 0x00) && (data[7] == 0x01) && (data[8] == 0x01)) // GVH5183 (UUID) 5183 B5183011
        {
            if (Model == ThermometerType::Unknown)
                Model = ThermometerType::H5183;
            // Govee Bluetooth Wireless Meat Thermometer, Digital Grill Thermometer with 1 Probe, 230ft Remote Temperature Monitor, Smart Kitchen Cooking Thermometer, Alert Notifications for BBQ, Oven, Smoker, Cakes
            // https://www.amazon.com/gp/product/B092ZTD96V
            // The probe measuring range is 0° to 300°C /32° to 572°F.
            // 5DA1B4 01000101 E4 01 80 0708 13 24 00 00
            // 2 3 4  5 6 7 8  9  0  1  2 3  4  5  6  7
            // (Manu) 5DA1B4 01000101 81 0180 07D0 1324 0000 (Temp) 20°C (Temp) 49°C (Battery) 1% (Other: 00)  (Other: 00)  (Other: 00)  (Other: 00)  (Other: 00)  (Other: BF)
            // the first three bytes are the last three bytes of the bluetooth address.
            // then next four bytes appear to be a signature for the device type.
            // Model = ThermometerType::H5181;
            // Govee Bluetooth Meat Thermometer, 230ft Range Wireless Grill Thermometer Remote Monitor with Temperature Probe Digital Grilling Thermometer with Smart Alerts for Smoker Cooking BBQ Kitchen Oven
            // https://www.amazon.com/dp/B092ZTJW37/
            short iTemp = short(data[12]) << 8 | short(data[13]);
            Temperature[0] = float(iTemp) / 100.0;
            iTemp = short(data[14]) << 8 | short(data[15]);
            Temperature[1] = float(iTemp) / 100.0; // This appears to be the alarm temperature.
            Humidity = 0;
            Battery = int(data[9] & 0x7F);
            Averages = 1;
            time(&Time);
            for (auto index = 0; index < (sizeof(Temperature) / sizeof(Temperature[0])); index++)
                TemperatureMin[index] = TemperatureMax[index] = Temperature[index];	//HACK: make sure that these values are set
            rval = true;
        }
        else if (data_len == 20 && (data[5] == 0x01) && (data[6] == 0x00) && (data[7] == 0x01) && (data[8] == 0x01)) // GVH5182 (UUID) 5182 (Manu) 30132701000101E4018606A413F78606A41318
        {
            if (Model == ThermometerType::Unknown)
                Model = ThermometerType::H5182;
            // Govee Bluetooth Meat Thermometer, 230ft Range Wireless Grill Thermometer Remote Monitor with Temperature Probe Digital Grilling Thermometer with Smart Alerts for Smoker , Cooking, BBQ, Kitchen, Oven
            // https://www.amazon.com/gp/product/B094N2FX9P
            // 301327 01000101 64 01 80 05DC 1324 86 06A4 FFFF
            // 2 3 4  5 6 7 8  9  0  1  2 3  4 5  6  7 8  9 0
            // (Manu) 301327 01000101 3A 01 86 076C FFFF 86 076C FFFF (Temp) 19°C (Temp) -0.01°C (Temp) 19°C (Temp) -0.01°C (Battery) 58%
            // If the probe is not connected to the device, the temperature data is set to FFFF.
            // If the alarm is not set for the probe, the data is set to FFFF.
            short iTemp = short(data[12]) << 8 | short(data[13]);	// Probe 1 Temperature
            Temperature[0] = float(iTemp) / 100.0;
            iTemp = short(data[14]) << 8 | short(data[15]);			// Probe 1 Alarm Temperature
            Temperature[1] = float(iTemp) / 100.0;
            iTemp = short(data[17]) << 8 | short(data[18]);			// Probe 2 Temperature
            Temperature[2] = float(iTemp) / 100.0;
            iTemp = short(data[19]) << 8 | short(data[20]);			// Probe 2 Alarm Temperature
            Temperature[3] = float(iTemp) / 100.0;
            Humidity = 0;
            Battery = int(data[9]);
            Averages = 1;
            time(&Time);
            for (auto index = 0; index < (sizeof(Temperature) / sizeof(Temperature[0])); index++)
                TemperatureMin[index] = TemperatureMax[index] = Temperature[index];	//HACK: make sure that these values are set
            rval = true;
        }
    }
    return(rval);
}
*/

std::string TypeToString(int type)
{
    std::string type_str;
    switch (type) {
    case DBUS_TYPE_INVALID:
        type_str = "Invalid";
        break;
    case DBUS_TYPE_VARIANT:
        type_str = "Variant";
        break;
    case DBUS_TYPE_ARRAY:
        type_str = "Array";
        break;
    case DBUS_TYPE_BYTE:
        type_str = "Byte";
        break;
    case DBUS_TYPE_BOOLEAN:
        type_str = "Boolean";
        break;
    case DBUS_TYPE_INT16:
        type_str = "Int16";
        break;
    case DBUS_TYPE_UINT16:
        type_str = "UInt16";
        break;
    case DBUS_TYPE_INT32:
        type_str = "Int32";
        break;
    case DBUS_TYPE_UINT32:
        type_str = "UInt32";
        break;
    case DBUS_TYPE_INT64:
        type_str = "Int64";
        break;
    case DBUS_TYPE_UINT64:
        type_str = "UInt64";
        break;
    case DBUS_TYPE_DOUBLE:
        type_str = "Double";
        break;
    case DBUS_TYPE_STRING:
        type_str = "String";
        break;
    case DBUS_TYPE_OBJECT_PATH:
        type_str = "ObjectPath";
        break;
    case DBUS_TYPE_SIGNATURE:
        type_str = "Signature";
        break;
    case DBUS_TYPE_STRUCT:
        type_str = "Struct";
        break;
    case DBUS_TYPE_DICT_ENTRY:
        type_str = "DictEntry";
        break;
    default:
        type_str = "Unknown variant: " + std::to_string(type);
        break;
    }

    return type_str;
}

template< typename T >
std::string int_to_hex(T i)
{
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(sizeof(T) * 2)
        << std::hex << i;
    return stream.str();
}

std::string ConvertStringToHexString(const std::string& integer_string)
{
    long integer = std::strtol(integer_string.c_str(), nullptr, 10);
    return int_to_hex((uint16_t)integer);
}

std::string ConvertByteToHexString(unsigned char byte)
{
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    return ss.str();
}

/*
https://dbus.freedesktop.org/doc/dbus-monitor.1.html
sudo dbus-monitor --system --monitor path="/org/bluez/hci0"

method call time=1721846945.917316 sender=:1.3349 -> destination=org.bluez serial=8 path=/org/bluez/hci0; interface=org.freedesktop.DBus.Properties; member=Set
   string "org.bluez.Adapter1"
   string "Powered"
   variant       boolean true
method call time=1721846945.918393 sender=:1.3349 -> destination=org.bluez serial=9 path=/org/bluez/hci0; interface=org.bluez.Adapter1; member=SetDiscoveryFilter
   array [
      dict entry(
         string "Transport"
         variant             string "le"
      )
      dict entry(
         string "DuplicateData"
         variant             boolean true
      )
   ]
*/

//std::string dbus_message_to_string(DBusMessage* message)
//{
//    // This function is currently a waste of space. I thought it was a simple way of converting a message to human readable content, but the example I got it from seems to be a special case
//    // https://github.com/makercrew/dbus-sample?tab=readme-ov-file#my-baby-just-a-wrote-me-a-letter
//    std::string rval;
//    DBusError dbus_error;
//    dbus_error_init(&dbus_error); // Initialize D-Bus error
//    const char* dbus_result(nullptr);
//    // Parse response
//    dbus_message_get_args(message, &dbus_error, DBUS_TYPE_STRING, &dbus_result, DBUS_TYPE_INVALID);
//    if (dbus_result != nullptr)
//        rval = dbus_result;
//    else if (dbus_error_is_set(&dbus_error))
//        rval = dbus_error.message;
//    dbus_error_free(&dbus_error);
//    return(rval);
//}

void bluez_power_on(DBusConnection* dbus_conn, const char* adapter_path, const bool PowerOn = true)
{

    // This was hacked from looking at https://git.kernel.org/pub/scm/network/connman/connman.git/tree/gdbus/client.c#n667
    // https://www.mankier.com/5/org.bluez.Adapter#Interface-boolean_Powered_%5Breadwrite%5D
    DBusMessage* dbus_msg = dbus_message_new_method_call("org.bluez", adapter_path, "org.freedesktop.DBus.Properties", "Set"); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga98ddc82450d818138ef326a284201ee0
    if (!dbus_msg)
        std::cout << "Can't allocate new method call" << std::endl;
    else
    {
        DBusMessageIter iterParameter;
        dbus_message_iter_init_append(dbus_msg, &iterParameter); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaf733047c467ce21f4a53b65a388f1e9d
        const char* adapter = "org.bluez.Adapter1";
        dbus_message_iter_append_basic(&iterParameter, DBUS_TYPE_STRING, &adapter); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga17491f3b75b3203f6fc47dcc2e3b221b
        const char* powered = "Powered";
        dbus_message_iter_append_basic(&iterParameter, DBUS_TYPE_STRING, &powered);
        DBusMessageIter variant;
        dbus_message_iter_open_container(&iterParameter, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga943150f4e87fd8507da224d22c266100
        dbus_bool_t cpTrue = PowerOn ? TRUE : FALSE;
        dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &cpTrue);
        dbus_message_iter_close_container(&iterParameter, &variant); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaf00482f63d4af88b7851621d1f24087a
        dbus_connection_send(dbus_conn, dbus_msg, NULL); // https://dbus.freedesktop.org/doc/api/html/group__DBusConnection.html#gae1cb64f4cf550949b23fd3a756b2f7d0
        std::cout << __FILE__ << "(" << __LINE__ << "): " << dbus_message_get_path(dbus_msg) << ": " << dbus_message_get_interface(dbus_msg) << ": " << dbus_message_get_member(dbus_msg) << powered << ": " << std::boolalpha << PowerOn << std::endl;
        dbus_message_unref(dbus_msg); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gab69441efe683918f6a82469c8763f464
    }
}
// These three helped me figure out how to send an array of DICT entries
// https://stackoverflow.com/questions/29973486/d-bus-how-to-create-and-send-a-dict
// https://git.kernel.org/pub/scm/network/connman/connman.git/tree/gdbus/client.c#n667
// https://android.googlesource.com/platform/external/wpa_supplicant_8/+/master/wpa_supplicant/dbus/dbus_dict_helpers.c

void bluez_filter_le(DBusConnection* dbus_conn, const char* adapter_path, const bool DuplicateData = true, const bool bFilter = true)
{
    // https://www.mankier.com/5/org.bluez.Adapter#Interface-void_SetDiscoveryFilter(dict_filter)
    DBusMessage* dbus_msg = dbus_message_new_method_call("org.bluez", adapter_path, "org.bluez.Adapter1", "SetDiscoveryFilter");
    if (!dbus_msg)
        std::cout << "Can't allocate new method call" << std::endl;
    else
    {
        if (bFilter)
        {
            DBusMessageIter iterParameter;
            dbus_message_iter_init_append(dbus_msg, &iterParameter);
            DBusMessageIter iterArray;
            dbus_message_iter_open_container(&iterParameter, DBUS_TYPE_ARRAY, "{sv}", &iterArray);
            DBusMessageIter iterDict;
            dbus_message_iter_open_container(&iterArray, DBUS_TYPE_DICT_ENTRY, NULL, &iterDict);
            const char* cpTransport = "Transport";
            dbus_message_iter_append_basic(&iterDict, DBUS_TYPE_STRING, &cpTransport);
            DBusMessageIter iterVariant;
            dbus_message_iter_open_container(&iterDict, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &iterVariant);
            const char* cpBTLE = "le";
            dbus_message_iter_append_basic(&iterVariant, DBUS_TYPE_STRING, &cpBTLE);
            dbus_message_iter_close_container(&iterDict, &iterVariant);
            dbus_message_iter_close_container(&iterArray, &iterDict);
            dbus_message_iter_open_container(&iterArray, DBUS_TYPE_DICT_ENTRY, NULL, &iterDict);
            const char* cpDuplicateData = "DuplicateData";
            dbus_message_iter_append_basic(&iterDict, DBUS_TYPE_STRING, &cpDuplicateData);
            dbus_message_iter_open_container(&iterDict, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &iterVariant);
            dbus_bool_t cpTrue = DuplicateData ? TRUE : FALSE;
            dbus_message_iter_append_basic(&iterVariant, DBUS_TYPE_BOOLEAN, &cpTrue);
            dbus_message_iter_close_container(&iterDict, &iterVariant);
            dbus_message_iter_close_container(&iterArray, &iterDict);
            dbus_message_iter_close_container(&iterParameter, &iterArray);
        }
        else
        {
            DBusMessageIter iterParameter;
            dbus_message_iter_init_append(dbus_msg, &iterParameter);
            DBusMessageIter iterArray;
            dbus_message_iter_open_container(&iterParameter, DBUS_TYPE_ARRAY, "{}", &iterArray);
            DBusMessageIter iterDict;
            dbus_message_iter_open_container(&iterArray, DBUS_TYPE_DICT_ENTRY, NULL, &iterDict);
            dbus_message_iter_close_container(&iterArray, &iterDict);
            dbus_message_iter_close_container(&iterParameter, &iterArray);
        }
        // Initialize D-Bus error
        DBusError dbus_error;
        dbus_error_init(&dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusErrors.html#ga8937f0b7cdf8554fa6305158ce453fbe
        DBusMessage* dbus_reply = dbus_connection_send_with_reply_and_block(dbus_conn, dbus_msg, DBUS_TIMEOUT_INFINITE, &dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusConnection.html#ga8d6431f17a9e53c9446d87c2ba8409f0
        std::cout << __FILE__ << "(" << __LINE__ << "): " << dbus_message_get_path(dbus_msg) << ": " << dbus_message_get_interface(dbus_msg) << ": " << dbus_message_get_member(dbus_msg) << std::endl;
        if (!dbus_reply)
        {
            std::cout << __FILE__ << "(" << __LINE__ << "): Error: " << dbus_message_get_interface(dbus_msg) << ": " << dbus_message_get_member(dbus_msg);
            if (dbus_error_is_set(&dbus_error))
            {
                std::cout << ": " << dbus_error.message;
                dbus_error_free(&dbus_error);
            }
            std::cout << std::endl;
        }
        else
            dbus_message_unref(dbus_reply);
        dbus_message_unref(dbus_msg);
    }
}

bool bluez_discovery(DBusConnection* dbus_conn, const char* adapter_path, const bool bStartDiscovery = true)
{
    bool bStarted = false;
    // https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/adapter-api.txt
    DBusMessage* dbus_msg = dbus_message_new_method_call("org.bluez", adapter_path, "org.bluez.Adapter1", bStartDiscovery?"StartDiscovery":"StopDiscovery");
    DBusError dbus_error;
    dbus_error_init(&dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusErrors.html#ga8937f0b7cdf8554fa6305158ce453fbe
    DBusMessage* dbus_reply = dbus_connection_send_with_reply_and_block(dbus_conn, dbus_msg, DBUS_TIMEOUT_INFINITE, &dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusConnection.html#ga8d6431f17a9e53c9446d87c2ba8409f0
    std::cout << __FILE__ << "(" << __LINE__ << "): " << dbus_message_get_path(dbus_msg) << ": " << dbus_message_get_interface(dbus_msg) << ": " << dbus_message_get_member(dbus_msg) << std::endl;
    if (!dbus_reply)
    {
        std::cout << __FILE__ << "(" << __LINE__ << "): Error: " << dbus_message_get_interface(dbus_msg) << ": " << dbus_message_get_member(dbus_msg);
        if (dbus_error_is_set(&dbus_error))
        {
            std::cout << ": " << dbus_error.message;
            dbus_error_free(&dbus_error);
        }
        std::cout << std::endl;
    }
    else
    {
        bStarted = true;
        dbus_message_unref(dbus_reply);
    }
    dbus_message_unref(dbus_msg);
    return(bStarted);
}

void bluez_find_adapters(DBusConnection* dbus_conn, std::vector<std::string> &adapter_paths)
{
    // Initialize D-Bus error
    DBusError dbus_error;
    dbus_error_init(&dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusErrors.html#ga8937f0b7cdf8554fa6305158ce453fbe
    DBusMessage* dbus_msg = dbus_message_new_method_call("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
    if (!dbus_msg)
        std::cout << "Can't allocate new method call" << std::endl;
    else
    {
        dbus_error_init(&dbus_error);
        DBusMessage* dbus_reply = dbus_connection_send_with_reply_and_block(dbus_conn, dbus_msg, DBUS_TIMEOUT_USE_DEFAULT, &dbus_error);
        std::cout << __FILE__ << "(" << __LINE__ << "): " << dbus_message_get_path(dbus_msg) << ": " << dbus_message_get_interface(dbus_msg) << ": " << dbus_message_get_member(dbus_msg) << std::endl;
        dbus_message_unref(dbus_msg);
        if (!dbus_reply)
        {
            std::cout << "Can't get bluez managed objects:" << std::endl;
            if (dbus_error_is_set(&dbus_error))
            {
                std::cout << dbus_error.message << std::endl;
                dbus_error_free(&dbus_error);
            }
        }
        else
        {
            // expected reply in the form of: "a{oa{sa{sv}}}"
            /*
            wim@WimPi5:~ $ dbus-send --system --dest=org.bluez --print-reply / org.freedesktop.DBus.ObjectManager.GetManagedObjects
            method return time=1721945091.093473 sender=:1.436 -> destination=:1.438 serial=99 reply_serial=2
               array [
                  dict entry(
                     object path "/org/bluez"
                     array [
                        dict entry(
                           string "org.freedesktop.DBus.Introspectable"
                           array [
                           ]
                        )
                        dict entry(
                           string "org.bluez.AgentManager1"
                           array [
                           ]
                        )
                        dict entry(
                           string "org.bluez.ProfileManager1"
                           array [
                           ]
                        )
                        dict entry(
                           string "org.bluez.HealthManager1"
                           array [
                           ]
                        )
                     ]
                  )
                  dict entry(
                     object path "/org/bluez/hci0"
                     array [
                        dict entry(
                           string "org.freedesktop.DBus.Introspectable"
                           array [
                           ]
                        )
                        dict entry(
                           string "org.bluez.Adapter1"
                           array [
                              dict entry(
                                 string "Address"
                                 variant                         string "2C:CF:67:0B:78:71"
                              )
                              dict entry(
                                 string "AddressType"
                                 variant                         string "public"
                              )
                              dict entry(
                                 string "Name"
                                 variant                         string "WimPi5"
                              )
                              dict entry(
                                 string "Alias"
                                 variant                         string "WimPi5"
                              )
                              dict entry(
                                 string "Class"
                                 variant                         uint32 7077888
                              )
                              dict entry(
                                 string "Powered"
                                 variant                         boolean true
                              )
                              dict entry(
                                 string "Discoverable"
                                 variant                         boolean false
                              )
                              dict entry(
                                 string "DiscoverableTimeout"
                                 variant                         uint32 180
                              )
                              dict entry(
                                 string "Pairable"
                                 variant                         boolean true
                              )
                              dict entry(
                                 string "PairableTimeout"
                                 variant                         uint32 0
                              )
                              dict entry(
                                 string "Discovering"
                                 variant                         boolean false
                              )
                              dict entry(
                                 string "UUIDs"
                                 variant                         array [
                                       string "0000110e-0000-1000-8000-00805f9b34fb"
                                       string "0000111f-0000-1000-8000-00805f9b34fb"
                                       string "00001200-0000-1000-8000-00805f9b34fb"
                                       string "0000110b-0000-1000-8000-00805f9b34fb"
                                       string "0000110a-0000-1000-8000-00805f9b34fb"
                                       string "0000110c-0000-1000-8000-00805f9b34fb"
                                       string "00001800-0000-1000-8000-00805f9b34fb"
                                       string "00001801-0000-1000-8000-00805f9b34fb"
                                       string "0000180a-0000-1000-8000-00805f9b34fb"
                                       string "0000111e-0000-1000-8000-00805f9b34fb"
                                    ]
                              )
                              dict entry(
                                 string "Modalias"
                                 variant                         string "usb:v1D6Bp0246d0542"
                              )
                              dict entry(
                                 string "Roles"
                                 variant                         array [
                                       string "central"
                                       string "peripheral"
                                    ]
                              )
                           ]
                        )
                        dict entry(
                           string "org.freedesktop.DBus.Properties"
                           array [
                           ]
                        )
                        dict entry(
                           string "org.bluez.GattManager1"
                           array [
                           ]
                        )
                        dict entry(
                           string "org.bluez.Media1"
                           array [
                              dict entry(
                                 string "SupportedUUIDs"
                                 variant                         array [
                                       string "0000110a-0000-1000-8000-00805f9b34fb"
                                       string "0000110b-0000-1000-8000-00805f9b34fb"
                                    ]
                              )
                           ]
                        )
                        dict entry(
                           string "org.bluez.NetworkServer1"
                           array [
                           ]
                        )
                        dict entry(
                           string "org.bluez.LEAdvertisingManager1"
                           array [
                              dict entry(
                                 string "ActiveInstances"
                                 variant                         byte 0
                              )
                              dict entry(
                                 string "SupportedInstances"
                                 variant                         byte 5
                              )
                              dict entry(
                                 string "SupportedIncludes"
                                 variant                         array [
                                       string "tx-power"
                                       string "appearance"
                                       string "local-name"
                                    ]
                              )
                           ]
                        )
                     ]
                  )
               ]
            */
            if (dbus_message_get_type(dbus_reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN)
            {
                int indent(16);
                std::cout << std::right << std::setw(indent) << "Message Type: " << std::string(dbus_message_type_to_string(dbus_message_get_type(dbus_reply))) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaed63e4c2baaa50d782e8ebb7643def19
                const std::string dbus_reply_Signature(dbus_message_get_signature(dbus_reply));
                std::cout << std::right << std::setw(indent) << "Signature: " << dbus_reply_Signature << std::endl;
                std::cout << std::right << std::setw(indent) << "Destination: " << std::string(dbus_message_get_destination(dbus_reply)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaed63e4c2baaa50d782e8ebb7643def19
                std::cout << std::right << std::setw(indent) << "Sender: " << std::string(dbus_message_get_sender(dbus_reply)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaed63e4c2baaa50d782e8ebb7643def19
                if (NULL != dbus_message_get_path(dbus_reply)) std::cout << std::right << std::setw(indent) << "Path: " << std::string(dbus_message_get_path(dbus_reply)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga18adf731bb42d324fe2624407319e4af
                if (NULL != dbus_message_get_interface(dbus_reply)) std::cout << std::right << std::setw(indent) << "Interface: " << std::string(dbus_message_get_interface(dbus_reply)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga1ad192bd4538cae556121a71b4e09d42
                if (NULL != dbus_message_get_member(dbus_reply)) std::cout << std::right << std::setw(indent) << "Member: " << std::string(dbus_message_get_member(dbus_reply)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaf5c6b705c53db07a5ae2c6b76f230cf9
                if (NULL != dbus_message_get_container_instance(dbus_reply)) std::cout << std::right << std::setw(indent) << "Container Instance: " << std::string(dbus_message_get_container_instance(dbus_reply)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaed63e4c2baaa50d782e8ebb7643def19
                if (!dbus_reply_Signature.compare("a{oa{sa{sv}}}"))
                {
                    DBusMessageIter root_iter;
                    dbus_message_iter_init(dbus_reply, &root_iter);
                    do {
                        DBusMessageIter array1_iter;
                        dbus_message_iter_recurse(&root_iter, &array1_iter);
                        do {
                            DBusMessageIter dict1_iter;
                            std::string dict1_object_path;
                            dbus_message_iter_recurse(&array1_iter, &dict1_iter);
                            do {
                                indent += 4;
                                if (DBUS_TYPE_OBJECT_PATH == dbus_message_iter_get_arg_type(&dict1_iter))
                                {
                                    DBusBasicValue value;
                                    dbus_message_iter_get_basic(&dict1_iter, &value);
                                    dict1_object_path = std::string(value.str);
                                    std::cout << std::right << std::setw(indent) << "Object Path: " << dict1_object_path << std::endl;
                                }
                                else if (DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&dict1_iter))
                                {
                                    DBusMessageIter array2_iter;
                                    dbus_message_iter_recurse(&dict1_iter, &array2_iter);
                                    do
                                    {
                                        DBusMessageIter dict2_iter;
                                        dbus_message_iter_recurse(&array2_iter, &dict2_iter);
                                        do
                                        {
                                            if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&dict2_iter))
                                            {
                                                DBusBasicValue value;
                                                dbus_message_iter_get_basic(&dict2_iter, &value);
                                                std::string val(value.str);
                                                std::cout << std::right << std::setw(indent) << "String: " << val << std::endl;
                                                if (!val.compare("org.bluez.Adapter1"))
                                                    adapter_paths.push_back(dict1_object_path);
                                            }
                                        } while (dbus_message_iter_next(&dict2_iter));
                                    } while (dbus_message_iter_next(&array2_iter));
                                }
                                else
                                    std::cout << std::right << std::setw(indent) << "Unexpected type in message: " << TypeToString(dbus_message_iter_get_arg_type(&dict1_iter)) << std::endl;
                                indent -= 4;
                            } while (dbus_message_iter_next(&dict1_iter));
                        } while (dbus_message_iter_next(&array1_iter));
                    } while (dbus_message_iter_next(&root_iter));
                }
            }
            dbus_message_unref(dbus_reply);
        }
    }
}

void bluez_dbus_msg_InterfacesAdded(DBusMessage* dbus_msg)
{
    if (std::string(dbus_message_get_signature(dbus_msg)).compare("oa{sa{sv}}"))
        std::cout << "Invalid Signature!!!";
    else
    {
        int indent(20);
        DBusMessageIter root_iter;
        std::string root_object_path;
        dbus_message_iter_init(dbus_msg, &root_iter);
        DBusBasicValue value;
        dbus_message_iter_get_basic(&root_iter, &value);
        root_object_path = std::string(value.str);
        std::cout << __FILE__ << "(" << __LINE__ << "): " << std::right << std::setw(indent) << "Object Path: " << root_object_path << std::endl;
        dbus_message_iter_next(&root_iter);
        DBusMessageIter array1_iter;
        dbus_message_iter_recurse(&root_iter, &array1_iter);
        do
        {
            DBusMessageIter dict1_iter;
            dbus_message_iter_recurse(&array1_iter, &dict1_iter);
            indent += 4;
            DBusBasicValue value;
            dbus_message_iter_get_basic(&dict1_iter, &value);
            std::string val(value.str);
            std::cout << __FILE__ << "(" << __LINE__ << "): " << std::right << std::setw(indent) << "String: " << val << std::endl;
            if (!val.compare("org.bluez.Device1"))
            {
                dbus_message_iter_next(&dict1_iter);
                DBusMessageIter array2_iter;
                dbus_message_iter_recurse(&dict1_iter, &array2_iter);
                do
                {
                    DBusMessageIter dict2_iter;
                    dbus_message_iter_recurse(&array2_iter, &dict2_iter);
                    indent += 4;
                    DBusBasicValue value;
                    dbus_message_iter_get_basic(&dict2_iter, &value);
                    std::string Key(value.str);
                    std::cout << __FILE__ << "(" << __LINE__ << "): " << std::right << std::setw(indent) << Key << ": ";
                    dbus_message_iter_next(&dict2_iter);
                    DBusMessageIter variant_iter;
                    dbus_message_iter_recurse(&dict2_iter, &variant_iter);
                    do
                    {
                        auto dbus_message_Type = dbus_message_iter_get_arg_type(&variant_iter);
                        if (dbus_type_is_basic(dbus_message_Type))
                        {
                            DBusBasicValue value;
                            dbus_message_iter_get_basic(&variant_iter, &value);
                            if ((DBUS_TYPE_STRING == dbus_message_Type) || (DBUS_TYPE_OBJECT_PATH == dbus_message_Type))
                                std::cout << value.str;
                            else if (DBUS_TYPE_BYTE == dbus_message_Type)
                                std::cout << std::to_string(value.byt);
                            else if (DBUS_TYPE_BOOLEAN == dbus_message_Type)
                                std::cout << std::to_string(value.bool_val);
                            else if (DBUS_TYPE_INT16 == dbus_message_Type)
                                std::cout << std::to_string(value.i16);
                            else if (DBUS_TYPE_UINT16 == dbus_message_Type)
                                std::cout << std::to_string(value.u16);
                            else if (DBUS_TYPE_INT32 == dbus_message_Type)
                                std::cout << std::to_string(value.i32);
                            else if (DBUS_TYPE_UINT32 == dbus_message_Type)
                                std::cout << std::to_string(value.u32);
                            else if (DBUS_TYPE_INT64 == dbus_message_Type)
                                std::cout << std::to_string(value.i64);
                            else if (DBUS_TYPE_UINT64 == dbus_message_Type)
                                std::cout << std::to_string(value.u64);
                            else if (DBUS_TYPE_DOUBLE == dbus_message_Type)
                                std::cout << std::to_string(value.dbl);
                            else if (DBUS_TYPE_DOUBLE == dbus_message_Type)
                                std::cout << std::to_string(value.dbl);
                            else
                                std::cout << "Unexpected basic type in variant";
                            std::cout << " (" << TypeToString(dbus_message_Type) << ")";
                        }
                        else if (DBUS_TYPE_ARRAY == dbus_message_Type)
                        {
                            if (!Key.compare("ManufacturerData"))
                            {
                                DBusMessageIter array3_iter;
                                dbus_message_iter_recurse(&variant_iter, &array3_iter);
                                do
                                {
                                    if (DBUS_TYPE_DICT_ENTRY == dbus_message_iter_get_arg_type(&array3_iter))
                                    { 
                                        std::cout << " (DBUS_TYPE_DICT_ENTRY){";
                                        DBusMessageIter dict1_iter;
                                        dbus_message_iter_recurse(&array3_iter, &dict1_iter);
                                        if (DBUS_TYPE_UINT16 == dbus_message_iter_get_arg_type(&dict1_iter))
                                        {
                                            DBusBasicValue value;
                                            dbus_message_iter_get_basic(&dict1_iter, &value);
                                            std::ios oldState(nullptr);
                                            oldState.copyfmt(std::cout);
                                            std::cout << " (UINT16: " << std::setw(4) << std::setfill('0') << std::hex << value.u16 << ")";
                                            std::cout.copyfmt(oldState);
                                        }
                                        else
                                            std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&dict1_iter)) << ")";
                                        dbus_message_iter_next(&dict1_iter);
                                        if (DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&dict1_iter))
                                        {
                                            std::cout << " (DBUS_TYPE_VARIANT)[";
                                            DBusMessageIter variant2_iter;
                                            dbus_message_iter_recurse(&dict1_iter, &variant2_iter);
                                            if (DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&variant2_iter))
                                            { 
                                                std::cout << " (DBUS_TYPE_ARRAY){";
                                                DBusMessageIter array4_iter;
                                                dbus_message_iter_recurse(&variant2_iter, &array4_iter);
                                                do
                                                {
                                                    if (DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&array4_iter))
                                                    {
                                                        std::cout << " (DBUS_TYPE_VARIANT)[";
                                                        DBusMessageIter variant3_iter;
                                                        dbus_message_iter_recurse(&dict1_iter, &variant3_iter);
                                                        std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&variant3_iter)) << ")";
                                                        std::cout << " ]";
                                                    }
                                                    else if (DBUS_TYPE_BYTE == dbus_message_iter_get_arg_type(&array4_iter))
                                                    {
                                                        DBusBasicValue value;
                                                        dbus_message_iter_get_basic(&array4_iter, &value);
                                                        std::ios oldState(nullptr);
                                                        oldState.copyfmt(std::cout);
                                                        std::cout << std::setw(2) << std::setfill('0') << std::hex << int(value.byt);
                                                        std::cout.copyfmt(oldState);
                                                    }
                                                    else
                                                        std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&array4_iter)) << ")";
                                                } while (dbus_message_iter_next(&array4_iter));
                                                std::cout << "}";
                                            }
                                            else
                                                std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&variant2_iter)) << ")";
                                            std::cout << " ]";
                                        }
                                        else
                                            std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&dict1_iter)) << ")";
                                        std::cout << " }";
                                    }
                                    else
                                        std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&array3_iter)) << ")";
                                } while (dbus_message_iter_next(&array3_iter));
                            }
                            else if (!Key.compare("UUIDs"))
                            {
                                DBusMessageIter array3_iter;
                                dbus_message_iter_recurse(&variant_iter, &array3_iter);
                                do
                                {
                                    if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&array3_iter))
                                    { 
                                        DBusBasicValue value;
                                        dbus_message_iter_get_basic(&array3_iter, &value);
                                        std::cout << " " << value.str;
                                    }
                                    else
                                        std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&array3_iter)) << ")";
                                } while (dbus_message_iter_next(&array3_iter));
                            }
                            else
                                std::cout << "TODO: Array Decode";
                        }
                        else
                            std::cout << "Unexpected type in variant (" << TypeToString(dbus_message_Type) << ")";
                        std::cout << std::endl;
                    } while (dbus_message_iter_next(&variant_iter));
                    indent -= 4;
                } while (dbus_message_iter_next(&array2_iter));
            }
            indent -= 4;
        } while (dbus_message_iter_next(&array1_iter));
    }
}

void bluez_dbus_msg_PropertiesChanged(DBusMessage* dbus_msg)
{
    if (std::string(dbus_message_get_signature(dbus_msg)).compare("sa{sv}as"))
        std::cout << "Invalid Signature!!!";
    else
    {
        int indent(20);
        DBusMessageIter root_iter;
        std::string root_object_path;
        dbus_message_iter_init(dbus_msg, &root_iter);
        DBusBasicValue value;
        dbus_message_iter_get_basic(&root_iter, &value);
        root_object_path = std::string(value.str);
        std::cout << __FILE__ << "(" << __LINE__ << "): " << std::right << std::setw(indent) << "Object Path: " << root_object_path << std::endl;
        dbus_message_iter_next(&root_iter);
        DBusMessageIter array1_iter;
        dbus_message_iter_recurse(&root_iter, &array1_iter);
        do
        {
            DBusMessageIter dict1_iter;
            dbus_message_iter_recurse(&array1_iter, &dict1_iter); // The key of the dict
            indent += 4;
            DBusBasicValue value;
            dbus_message_iter_get_basic(&dict1_iter, &value);
            std::string Key(value.str);
            std::cout << __FILE__ << "(" << __LINE__ << "): " << std::right << std::setw(indent) << Key << ": ";
            dbus_message_iter_next(&dict1_iter);
            DBusMessageIter variant_iter;
            dbus_message_iter_recurse(&dict1_iter, &variant_iter); // the Variant value of the dict
            do // process the variant in the dict
            {
                auto dbus_message_Type = dbus_message_iter_get_arg_type(&variant_iter);
                if (dbus_type_is_basic(dbus_message_Type))
                {
                    DBusBasicValue value;
                    dbus_message_iter_get_basic(&variant_iter, &value);
                    if ((DBUS_TYPE_STRING == dbus_message_Type) || (DBUS_TYPE_OBJECT_PATH == dbus_message_Type))
                        std::cout << value.str;
                    else if (DBUS_TYPE_BYTE == dbus_message_Type)
                        std::cout << std::to_string(value.byt);
                    else if (DBUS_TYPE_BOOLEAN == dbus_message_Type)
                        std::cout << std::to_string(value.bool_val);
                    else if (DBUS_TYPE_INT16 == dbus_message_Type)
                        std::cout << std::to_string(value.i16);
                    else if (DBUS_TYPE_UINT16 == dbus_message_Type)
                        std::cout << std::to_string(value.u16);
                    else if (DBUS_TYPE_INT32 == dbus_message_Type)
                        std::cout << std::to_string(value.i32);
                    else if (DBUS_TYPE_UINT32 == dbus_message_Type)
                        std::cout << std::to_string(value.u32);
                    else if (DBUS_TYPE_INT64 == dbus_message_Type)
                        std::cout << std::to_string(value.i64);
                    else if (DBUS_TYPE_UINT64 == dbus_message_Type)
                        std::cout << std::to_string(value.u64);
                    else if (DBUS_TYPE_DOUBLE == dbus_message_Type)
                        std::cout << std::to_string(value.dbl);
                    else if (DBUS_TYPE_DOUBLE == dbus_message_Type)
                        std::cout << std::to_string(value.dbl);
                    else
                        std::cout << "Unexpected basic type in variant";
                    std::cout << " (" << TypeToString(dbus_message_Type) << ")";
                }
                else if (DBUS_TYPE_ARRAY == dbus_message_Type)
                {
                    if (!Key.compare("ManufacturerData"))
                    {
                        DBusMessageIter array3_iter;
                        dbus_message_iter_recurse(&variant_iter, &array3_iter);
                        do
                        {
                            if (DBUS_TYPE_DICT_ENTRY == dbus_message_iter_get_arg_type(&array3_iter))
                            {
                                std::cout << " (DBUS_TYPE_DICT_ENTRY){";
                                DBusMessageIter dict1_iter;
                                dbus_message_iter_recurse(&array3_iter, &dict1_iter);
                                if (DBUS_TYPE_UINT16 == dbus_message_iter_get_arg_type(&dict1_iter))
                                {
                                    DBusBasicValue value;
                                    dbus_message_iter_get_basic(&dict1_iter, &value);
                                    std::ios oldState(nullptr);
                                    oldState.copyfmt(std::cout);
                                    std::cout << " (UINT16: " << std::setw(4) << std::setfill('0') << std::hex << value.u16 << ")";
                                    std::cout.copyfmt(oldState);
                                }
                                else
                                    std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&dict1_iter)) << ")";
                                dbus_message_iter_next(&dict1_iter);
                                if (DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&dict1_iter))
                                {
                                    std::cout << " (DBUS_TYPE_VARIANT)[";
                                    DBusMessageIter variant2_iter;
                                    dbus_message_iter_recurse(&dict1_iter, &variant2_iter);
                                    if (DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&variant2_iter))
                                    {
                                        std::cout << " (DBUS_TYPE_ARRAY){";
                                        DBusMessageIter array4_iter;
                                        dbus_message_iter_recurse(&variant2_iter, &array4_iter);
                                        do
                                        {
                                            if (DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&array4_iter))
                                            {
                                                std::cout << " (DBUS_TYPE_VARIANT)[";
                                                DBusMessageIter variant3_iter;
                                                dbus_message_iter_recurse(&dict1_iter, &variant3_iter);
                                                std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&variant3_iter)) << ")";
                                                std::cout << " ]";
                                            }
                                            else if (DBUS_TYPE_BYTE == dbus_message_iter_get_arg_type(&array4_iter))
                                            {
                                                DBusBasicValue value;
                                                dbus_message_iter_get_basic(&array4_iter, &value);
                                                std::ios oldState(nullptr);
                                                oldState.copyfmt(std::cout);
                                                std::cout << std::setw(2) << std::setfill('0') << std::hex << int(value.byt);
                                                std::cout.copyfmt(oldState);
                                            }
                                            else
                                                std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&array4_iter)) << ")";
                                        } while (dbus_message_iter_next(&array4_iter));
                                        std::cout << "}";
                                    }
                                    else
                                        std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&variant2_iter)) << ")";
                                    std::cout << " ]";
                                }
                                else
                                    std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&dict1_iter)) << ")";
                                std::cout << " }";
                            }
                            else
                                std::cout << " (" << TypeToString(dbus_message_iter_get_arg_type(&array3_iter)) << ")";
                        } while (dbus_message_iter_next(&array3_iter));
                    }
                    else
                        std::cout << "TODO: Array Decode";
                }
                else
                    std::cout << "Unexpected type in variant (" << TypeToString(dbus_message_Type) << ")";
            } while (dbus_message_iter_next(&variant_iter));
            std::cout << std::endl;
            indent -= 4;
        } while (dbus_message_iter_next(&array1_iter));
        //dbus_message_iter_next(&root_iter);
        //DBusMessageIter array2_iter;
        //dbus_message_iter_recurse(&root_iter, &array2_iter);
        //do
        //{
        //    DBusBasicValue value;
        //    dbus_message_iter_get_basic(&array2_iter, &value);
        //    std::string key(value.str);
        //    std::cout << std::right << std::setw(indent) << "String: " << key;
        //} while (dbus_message_iter_next(&array2_iter));
    }
}

int main(int argc, char* argv[])
{
    // Initialize D-Bus error
    DBusError dbus_error;
    dbus_error_init(&dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusErrors.html#ga8937f0b7cdf8554fa6305158ce453fbe

    // Connect to the system bus
    DBusConnection* dbus_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusBus.html#ga77ba5250adb84620f16007e1b023cf26
    if (dbus_error_is_set(&dbus_error)) // https://dbus.freedesktop.org/doc/api/html/group__DBusErrors.html#gab0ed62e9fc2685897eb2d41467c89405
    {
        std::cout << "Error connecting to the D-Bus system bus: " << dbus_error.message << std::endl;
        dbus_error_free(&dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusErrors.html#gaac6c14ead14829ee4e090f39de6a7568
    }
    else
    {
        std::cout << __FILE__ << "(" << __LINE__ << "): Connected to D-Bus as \"" << dbus_bus_get_unique_name(dbus_conn) << "\"" << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusBus.html#ga8c10339a1e62f6a2e5752d9c2270d37b
        std::vector<std::string> BlueZAdapters;
        bluez_find_adapters(dbus_conn, BlueZAdapters);
        for (auto& adapter : BlueZAdapters)
            std::cout << __FILE__ << "(" << __LINE__ << "): Bluetooth Adapter: " << adapter << std::endl;
        if (!BlueZAdapters.empty())
        {
            std::string BlueZAdapter(BlueZAdapters.front());
            bluez_power_on(dbus_conn, BlueZAdapter.c_str());
            bluez_filter_le(dbus_conn, BlueZAdapter.c_str());
            if (bluez_discovery(dbus_conn, BlueZAdapter.c_str(), true))
            {
                dbus_connection_flush(dbus_conn); // https://dbus.freedesktop.org/doc/api/html/group__DBusConnection.html#ga10e68d9d2f41d655a4151ddeb807ff54
                std::vector<std::string> MatchRules;
                MatchRules.push_back("type='signal',sender='org.bluez',member='InterfacesAdded'");
                MatchRules.push_back("type='signal',sender='org.bluez',member='PropertiesChanged'");
                for (auto& MatchRule : MatchRules)
                {
                    dbus_error_init(&dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusErrors.html#ga8937f0b7cdf8554fa6305158ce453fbe
                    dbus_bus_add_match(dbus_conn, MatchRule.c_str(), &dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusBus.html#ga4eb6401ba014da3dbe3dc4e2a8e5b3ef
                    if (dbus_error_is_set(&dbus_error))
                    {
                        std::cout << "Error adding a match rule on the D-Bus system bus: " << dbus_error.message << std::endl;
                        dbus_error_free(&dbus_error);
                    }
                }
                // Main loop
                bool running = true;
                while (running)
                {
                    // Wait for access to the D-Bus
                    if (!dbus_connection_read_write(dbus_conn, 1000)) // https://dbus.freedesktop.org/doc/api/html/group__DBusConnection.html#ga371163b4955a6e0bf0f1f70f38390c14
                    {
                        std::cout << "D-Bus connection was closed" << std::endl;
                        running = false;
                    }
                    else
                    {
                        // Pop first message on D-Bus connection
                        DBusMessage* dbus_msg = dbus_connection_pop_message(dbus_conn); // https://dbus.freedesktop.org/doc/api/html/group__DBusConnection.html#ga1e40d994ea162ce767e78de1c4988566

                        // If there is nothing to receive we get a NULL
                        if (dbus_msg != nullptr)
                        {
                            const std::string dbus_msg_Signature(dbus_message_get_signature(dbus_msg)); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaed63e4c2baaa50d782e8ebb7643def19
                            const std::string dbus_msg_Type(dbus_message_type_to_string(dbus_message_get_type(dbus_msg))); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga30c3cf672781da89cf9714e5ba761033
                            std::cout << __FILE__ << "(" << __LINE__ << "): " << "Got message:\tType: " << dbus_msg_Type << "\tSignature: \"" << dbus_msg_Signature << "\"" << std::endl;
                            if (DBUS_MESSAGE_TYPE_ERROR == dbus_message_get_type(dbus_msg))
                                std::cout << "  Error name: " << std::string(dbus_message_get_error_name(dbus_msg)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga4e98b2283707a8e0313fc7c6fe3b1b5f
                            else if (DBUS_MESSAGE_TYPE_SIGNAL == dbus_message_get_type(dbus_msg))
                            {
                                const std::string dbus_msg_Path(dbus_message_get_path(dbus_msg)); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga18adf731bb42d324fe2624407319e4af
                                const std::string dbus_msg_Member(dbus_message_get_member(dbus_msg)); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaf5c6b705c53db07a5ae2c6b76f230cf9
                                const std::string dbus_msg_Interface(dbus_message_get_interface(dbus_msg)); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga1ad192bd4538cae556121a71b4e09d42
                                //if (!dbus_msg_Path.compare("/org/bluez/hci0/dev_E3_8E_C8_C1_98_9A"))
                                {
                                    std::cout << __FILE__ << "(" << __LINE__ << "): " << "  Path: " << dbus_msg_Path << std::endl;
                                    std::cout << __FILE__ << "(" << __LINE__ << "): " << "  Interface: " << dbus_msg_Interface << std::endl;
                                    std::cout << __FILE__ << "(" << __LINE__ << "): " << "  Member: " << dbus_msg_Member << std::endl;
                                    // BLE advertisement messages arrive as InterfacesAdded messages
                                    if (!dbus_msg_Member.compare("InterfacesAdded"))
                                        bluez_dbus_msg_InterfacesAdded(dbus_msg);
                                    else if (!dbus_msg_Member.compare("PropertiesChanged"))
                                        bluez_dbus_msg_PropertiesChanged(dbus_msg);
                                    else if (!dbus_msg_Member.compare("NameAcquired"))
                                    {
                                        if (!dbus_msg_Signature.compare("s"))
                                        {
                                            DBusMessageIter root_iter;
                                            dbus_message_iter_init(dbus_msg, &root_iter);
                                            DBusBasicValue value;
                                            dbus_message_iter_get_basic(&root_iter, &value);
                                            const std::string dbus_msg_String(value.str);
                                            std::cout << __FILE__ << "(" << __LINE__ << "): " << "    String: " << dbus_msg_String << std::endl;
                                        }
                                    }
                                    else
                                        std::cout << "    Unhandled Member: " << dbus_msg_Member << std::endl;
                                }
                            }
                            // Free the message
                            dbus_message_unref(dbus_msg);
                        }
                    }
                }
                bluez_discovery(dbus_conn, BlueZAdapter.c_str(), false);
            }
            bluez_filter_le(dbus_conn, BlueZAdapter.c_str(), false, false); // remove discovery filter
        }
        // Close the connection
        // When using the System Bus, unreference the connection instead of closing it
        dbus_connection_unref(dbus_conn);
    }
    return 0;
}
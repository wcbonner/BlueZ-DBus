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
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Do this while running ble-dbus to see what else could be picked up from an advertisement message
// gdbus introspect --system --dest org.bluez --object-path /org/bluez --recurse


class Property
{
public:
    Property() = default;
    Property(const std::string& name_) { name = name_; };
    ~Property() = default;

    std::string name;
    std::string value;
};

static Property ble_properties[] = {
    {"Address"},
    {"Name"},
    //{"Icon"},
    //{"Class"},
    //{"UUIDs"},
    //{"Services"},
    //{"Paired"},
    //{"Connected"},
    //{"Trusted"},
    //{"Blocked"},
    {"Alias"},
    //{"Nodes"},
    //{"Adapter"},
    //{"LegacyPairing"},
    {"RSSI"},
    {"ServiceData"},
    {"ManufacturerData"}
    //{"TX"},
    //{"Broadcaster"}
};

#define InfoLog(_MESSAGE_) WriteLog((const char*)__FILE__, (int)__LINE__, std::string(_MESSAGE_))
#define ErrorLog(_MESSAGE_) WriteLog((const char*)__FILE__, (int)__LINE__, std::string(_MESSAGE_))

void WriteLog(const char* filename, int line_number, const std::string& message)
{
    std::string path = std::string(filename);
    std::string base_filename = path.substr(path.find_last_of("/\\") + 1); // Don't log path to file
    std::cout << base_filename << "(" << line_number << "): " << message << std::endl;
}

void Log(int indent, const std::string& message)
{
    for (int i = 0; i < indent; i++) {
        std::cout << " ";
    }
    std::cout << message << std::endl;
}

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

int TypeToString(DBusMessageIter* iter, std::string& type_str)
{
    if (iter == nullptr) {
        type_str = "nullptr";
        return DBUS_TYPE_INVALID;
    }

    int variant = dbus_message_iter_get_arg_type(iter);
    type_str = TypeToString(variant);

    return variant;
}

bool StartsWith(const std::string& str, const std::string& prefix)
{
    return (str.find(prefix) == 0);
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

std::string Join(const std::vector<std::string>& vec, const char* delim)
{
    std::string res;
    for (auto elm : vec) {
        if (res.empty()) {
            res = elm;
        }
        else {
            res = res + std::string(delim) + elm;
        }
    }
    return res;
}

bool HandleSimpleType(DBusMessageIter* iter, int indent, std::string& str_value)
{
    str_value = "";

    bool res = true;
    std::string type_str;
    int type = TypeToString(iter, type_str);
    Log(indent, "Simple type iter found: " + type_str);

    DBusBasicValue value;
    switch (type) {
    case DBUS_TYPE_OBJECT_PATH:
    case DBUS_TYPE_STRING:
        dbus_message_iter_get_basic(iter, &value);
        str_value = std::string(value.str);
        Log(indent, "Value: " + std::string(value.str));
        break;
    case DBUS_TYPE_BYTE:
        dbus_message_iter_get_basic(iter, &value);
        str_value = std::to_string(value.byt);
        Log(indent, "Value: " + std::to_string(value.byt));
        break;
    case DBUS_TYPE_BOOLEAN:
        dbus_message_iter_get_basic(iter, &value);
        str_value = std::to_string(value.bool_val);
        Log(indent, "Value: " + std::to_string(value.bool_val));
        break;
    case DBUS_TYPE_INT16:
        dbus_message_iter_get_basic(iter, &value);
        str_value = std::to_string(value.i16);
        Log(indent, "Value: " + std::to_string(value.i16));
        break;
    case DBUS_TYPE_UINT16:
        dbus_message_iter_get_basic(iter, &value);
        str_value = std::to_string(value.u16);
        Log(indent, "Value: " + std::to_string(value.u16));
        break;
    case DBUS_TYPE_INT32:
        dbus_message_iter_get_basic(iter, &value);
        str_value = std::to_string(value.i32);
        Log(indent, "Value: " + std::to_string(value.i32));
        break;
    case DBUS_TYPE_UINT32:
        dbus_message_iter_get_basic(iter, &value);
        str_value = std::to_string(value.u32);
        Log(indent, "Value: " + std::to_string(value.u32));
        break;
    case DBUS_TYPE_INT64:
        dbus_message_iter_get_basic(iter, &value);
        str_value = std::to_string(value.i64);
        Log(indent, "Value: " + std::to_string(value.i64));
        break;
    case DBUS_TYPE_UINT64:
        dbus_message_iter_get_basic(iter, &value);
        str_value = std::to_string(value.u64);
        Log(indent, "Value: " + std::to_string(value.u64));
        break;
    case DBUS_TYPE_DOUBLE:
        dbus_message_iter_get_basic(iter, &value);
        str_value = std::to_string(value.dbl);
        Log(indent, "Value: " + std::to_string(value.dbl));
        break;
    default:
        res = false;
        ErrorLog("Unexpected type: " + type_str);
        break;
    }

    return res;
}

std::vector<std::string> HandleArrayOfSomething(DBusMessageIter* iter, int type, int indent)
{
    std::vector<std::string> values;
    DBusMessageIter array_iter;
    dbus_message_iter_recurse(iter, &array_iter);
    do {
        DBusBasicValue value;
        dbus_message_iter_get_basic(&array_iter, &value);
        switch (type) {
        case DBUS_TYPE_OBJECT_PATH:
        case DBUS_TYPE_STRING:
            dbus_message_iter_get_basic(&array_iter, &value);
            Log(indent, "Array entry: " + std::string(value.str));
            values.push_back(std::string(value.str));
            break;
        case DBUS_TYPE_BYTE:
            dbus_message_iter_get_basic(&array_iter, &value);
            Log(indent, "Array entry: " + std::to_string(value.byt) + " - 0x" + ConvertByteToHexString(value.byt));
            values.push_back(ConvertByteToHexString(value.byt));
            break;
        case DBUS_TYPE_BOOLEAN:
            dbus_message_iter_get_basic(&array_iter, &value);
            Log(indent, "Array entry: " + std::to_string(value.bool_val));
            values.push_back(std::to_string(value.bool_val));
            break;
        case DBUS_TYPE_INT16:
            dbus_message_iter_get_basic(&array_iter, &value);
            Log(indent, "Array entry: " + std::to_string(value.i16));
            values.push_back(std::to_string(value.i16));
            break;
        case DBUS_TYPE_UINT16:
            dbus_message_iter_get_basic(&array_iter, &value);
            Log(indent, "Array entry: " + std::to_string(value.u16));
            values.push_back(std::to_string(value.u16));
            break;
        case DBUS_TYPE_INT32:
            dbus_message_iter_get_basic(&array_iter, &value);
            Log(indent, "Array entry: " + std::to_string(value.i32));
            values.push_back(std::to_string(value.i32));
            break;
        case DBUS_TYPE_UINT32:
            dbus_message_iter_get_basic(&array_iter, &value);
            Log(indent, "Array entry: " + std::to_string(value.u32));
            values.push_back(std::to_string(value.u32));
            break;
        case DBUS_TYPE_INT64:
            dbus_message_iter_get_basic(&array_iter, &value);
            Log(indent, "Array entry: " + std::to_string(value.i64));
            values.push_back(std::to_string(value.i64));
            break;
        case DBUS_TYPE_UINT64:
            dbus_message_iter_get_basic(&array_iter, &value);
            Log(indent, "Array entry: " + std::to_string(value.u64));
            values.push_back(std::to_string(value.u64));
            break;
        case DBUS_TYPE_DOUBLE:
            dbus_message_iter_get_basic(&array_iter, &value);
            Log(indent, "Array entry: " + std::to_string(value.dbl));
            values.push_back(std::to_string(value.dbl));
            break;
        default:
            ErrorLog("Unexpected array of type: " + TypeToString(type));
            break;
        }
    } while (dbus_message_iter_next(&array_iter));

    return values;
}

void HandleTargetDict(DBusMessageIter* iter, int indent, std::string property_name, std::string& str_value);

void HandleArrayValue(DBusMessageIter* iter, int indent, std::string property_name, std::string& str_value)
{
    int elm_count = dbus_message_iter_get_element_count(iter);
    Log(indent, "Elements in array: " + std::to_string(elm_count));

    int type = dbus_message_iter_get_element_type(iter);
    std::string type_str = TypeToString(type);
    Log(indent, "Elements are of type: " + type_str);
    indent += 2;

    str_value = "";

    std::vector<std::string> arr;
    if (elm_count > 0) {
        switch (type) {
        case DBUS_TYPE_DICT_ENTRY:
            // TODO MBE: This is now recursive
            HandleTargetDict(iter, indent, property_name, str_value);
            break;
        default:
            arr = HandleArrayOfSomething(iter, type, indent);
            str_value = Join(arr, "");
            break;
        }
    }
}

void HandleTargetVariant(DBusMessageIter* iter, int indent, std::string property_name, std::string& str_value)
{
    str_value = "";

    DBusMessageIter variant_iter;
    dbus_message_iter_recurse(iter, &variant_iter);
    do {
        std::string type_str;
        int type = TypeToString(&variant_iter, type_str);

        DBusBasicValue value;
        switch (type) {
        case DBUS_TYPE_OBJECT_PATH:
        case DBUS_TYPE_STRING:
            dbus_message_iter_get_basic(&variant_iter, &value);
            str_value = std::string(value.str);
            Log(indent, "Variant " + type_str + ": " + std::string(value.str));
            break;
        case DBUS_TYPE_BYTE:
            dbus_message_iter_get_basic(&variant_iter, &value);
            str_value = std::to_string(value.byt);
            Log(indent, "Variant " + type_str + ": " + std::to_string(value.byt));
            break;
        case DBUS_TYPE_BOOLEAN:
            dbus_message_iter_get_basic(&variant_iter, &value);
            str_value = std::to_string(value.bool_val);
            Log(indent, "Variant " + type_str + ": " + std::to_string(value.bool_val));
            break;
        case DBUS_TYPE_INT16:
            dbus_message_iter_get_basic(&variant_iter, &value);
            str_value = std::to_string(value.i16);
            Log(indent, "Variant " + type_str + ": " + std::to_string(value.i16));
            break;
        case DBUS_TYPE_UINT16:
            dbus_message_iter_get_basic(&variant_iter, &value);
            str_value = std::to_string(value.u16);
            Log(indent, "Variant " + type_str + ": " + std::to_string(value.u16));
            break;
        case DBUS_TYPE_INT32:
            dbus_message_iter_get_basic(&variant_iter, &value);
            str_value = std::to_string(value.i32);
            Log(indent, "Variant " + type_str + ": " + std::to_string(value.i32));
            break;
        case DBUS_TYPE_UINT32:
            dbus_message_iter_get_basic(&variant_iter, &value);
            str_value = std::to_string(value.u32);
            Log(indent, "Variant " + type_str + ": " + std::to_string(value.u32));
            break;
        case DBUS_TYPE_INT64:
            dbus_message_iter_get_basic(&variant_iter, &value);
            str_value = std::to_string(value.i64);
            Log(indent, "Variant " + type_str + ": " + std::to_string(value.i64));
            break;
        case DBUS_TYPE_UINT64:
            dbus_message_iter_get_basic(&variant_iter, &value);
            str_value = std::to_string(value.u64);
            Log(indent, "Variant " + type_str + ": " + std::to_string(value.u64));
            break;
        case DBUS_TYPE_DOUBLE:
            dbus_message_iter_get_basic(&variant_iter, &value);
            str_value = std::to_string(value.dbl);
            Log(indent, "Variant " + type_str + ": " + std::to_string(value.dbl));
            break;
        case DBUS_TYPE_ARRAY:
            HandleArrayValue(&variant_iter, indent, property_name, str_value);
            break;
        default:
            ErrorLog("Unexpected type in variant: " + type_str);
            break;
        }
    } while (dbus_message_iter_next(&variant_iter));
}

void HandleTargetArray(DBusMessageIter* iter, int indent, std::string property_name, std::string& str_value);

void HandleTargetDict(DBusMessageIter* iter, int indent, std::string property_name, std::string& str_value)
{
    str_value = "";

    std::string first;
    std::string second;

    DBusMessageIter dict_iter;
    dbus_message_iter_recurse(iter, &dict_iter);
    do {
        std::string type_str;
        int type = TypeToString(&dict_iter, type_str);
        //std::cout << "Dict iter found: " << type_str << std::endl;
        indent += 2;

        DBusBasicValue value;
        switch (type) {
        case DBUS_TYPE_STRING:
            dbus_message_iter_get_basic(&dict_iter, &value);
            str_value = std::string(value.str);
            Log(indent, "Dict entry: " + str_value);
            property_name = str_value;
            break;
        case DBUS_TYPE_VARIANT:
            HandleTargetVariant(&dict_iter, indent, property_name, str_value);
            break;
        case DBUS_TYPE_DICT_ENTRY:
            HandleTargetArray(&dict_iter, indent, property_name, str_value);
            break;
        default:
            ErrorLog("Unexpected type in dict: " + type_str);
            break;
        }
        //InfoLog(str_value);
        if (first.empty()) {
            first = str_value;
        }
        else if (second.empty()) {
            second = str_value;
        }
    } while (dbus_message_iter_next(&dict_iter));

    if (!first.empty() && !second.empty())
    {
        //InfoLog(first + ": " + second);
        for (auto& iter : ble_properties)
        {
            if (iter.name == first)
            {
                //InfoLog("Storing value for " + first);
                iter.value = second;
                break;
            }
        }
    }
}

void HandleTargetArray(DBusMessageIter* iter, int indent, std::string property_name, std::string& str_value)
{
    str_value = "";

    //int i = 0;
    std::string manufacturer_id;
    DBusMessageIter array_iter;
    dbus_message_iter_recurse(iter, &array_iter);
    do {
        std::string type_str;
        int type = TypeToString(&array_iter, type_str);
        //std::cout << "Array iter found: " << type_str << std::endl;

        switch (type) {
        case DBUS_TYPE_DICT_ENTRY:
            HandleTargetDict(&array_iter, indent, property_name, str_value);
            break;
        case DBUS_TYPE_VARIANT:
            HandleTargetVariant(&array_iter, indent, property_name, str_value);
            break;
        default:
            if (!HandleSimpleType(&array_iter, indent, str_value)) {
                ErrorLog("Unexpected type in target array: " + type_str);
            }
            break;
        }
        // Special handling of ManufacturerData
        if (property_name == "ManufacturerData" && manufacturer_id.empty()) {
            manufacturer_id = str_value;
        }
        //InfoLog("In array[" + std::to_string(i) + "]: " + str_value);
        //i++;
    } while (dbus_message_iter_next(&array_iter));

    // Special handling of ManufacturerData
    if (property_name == "ManufacturerData") {
        // Return Manufacturer + ManufacturerData as string of hex digits
        str_value = ConvertStringToHexString(manufacturer_id) + ":" + str_value;
    }
}

void HandleDict(DBusMessageIter* iter, const std::string& dict_entry_filter, int indent)
{
    DBusMessageIter dict_iter;
    dbus_message_iter_recurse(iter, &dict_iter);
    bool fail = false;
    do {
        std::string type_str;
        int type = TypeToString(&dict_iter, type_str);
        Log(indent, "Dict iter found: " + type_str);
        indent += 2;

        DBusBasicValue value;
        std::string val;
        switch (type) {
        case DBUS_TYPE_STRING:
            dbus_message_iter_get_basic(&dict_iter, &value);
            val = std::string(value.str);
            //Log(indent, "Dict entry: " + val);
            if (!StartsWith(val, dict_entry_filter)) {
                Log(indent, "Skipping dict entry: " + val + " due to filter mismatch");
                fail = true;
            }
            break;
        case DBUS_TYPE_ARRAY:
            HandleTargetArray(&dict_iter, indent, "", val);
            break;
        default:
            ErrorLog("Unexpected type in dict: " + type_str);
            break;
        }
    } while (!fail && dbus_message_iter_next(&dict_iter));
}

void HandleArray(DBusMessageIter* iter, const std::string& dict_entry_filter, int indent)
{
    DBusMessageIter array_iter;
    dbus_message_iter_recurse(iter, &array_iter);
    do {
        std::string type_str;
        int type = TypeToString(&array_iter, type_str);
        Log(indent, "Array iter found: " + type_str);
        indent += 2;

        switch (type) 
        {
        case DBUS_TYPE_DICT_ENTRY:
            HandleDict(&array_iter, dict_entry_filter, indent);
            break;
        case DBUS_TYPE_STRING:
            {
            DBusBasicValue value;
            dbus_message_iter_get_basic(&array_iter, &value);
            Log(indent, "String: " + std::string(value.str));
            //std::cout << std::right << std::setw(24) << "String: " << std::string(value.str) << std::endl;
            }
            break;
        default:
            ErrorLog("Unexpected type in array: " + type_str);
            break;
        }
    } while (dbus_message_iter_next(&array_iter));
}

void HandleSignalMessage(DBusMessage* dbus_msg, const std::string& object_path_filter, const std::string& dict_entry_filter, int indent)
{
    // Expect this signature: oa{sa{sv}} in an advertisement message
    // o: DBUS_TYPE_OBJECT_PATH
    // a: DBUS_TYPE_ARRAY
    if (!StartsWith(dbus_message_get_signature(dbus_msg), "oa")) {
        ErrorLog("Unexpected start of signature: " + std::string(dbus_message_get_signature(dbus_msg)) + ", expected 'oa'");
        return;
    }

    DBusMessageIter root_iter;
    dbus_message_iter_init(dbus_msg, &root_iter);
    bool fail = false;
    do 
    {
        std::string type_str;
        int type = TypeToString(&root_iter, type_str);

        DBusBasicValue value;
        std::string object_path;
        switch (type) 
        {
        case DBUS_TYPE_OBJECT_PATH: // Handle the 'o'
            dbus_message_iter_get_basic(&root_iter, &value);
            object_path = std::string(value.str);
            Log(indent, "Object Path: " + object_path);
            if (StartsWith(object_path, object_path_filter)) {
                Log(indent, "Object path filter match");
            }
            else {
                Log(indent, "Object path filter mismatch");
                fail = true;
            }
            break;
        case DBUS_TYPE_ARRAY: // Handle the 'a'
            HandleArray(&root_iter, dict_entry_filter, indent);
            break;
        default:
            Log(indent, "Unexpected type in message: " + type_str);
            fail = true;
            break;
        }
    } while (!fail && dbus_message_iter_next(&root_iter));
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

void bluez_power_on(DBusConnection* dbus_conn, const char* adapter_path = "/org/bluez/hci0", const bool PowerOn = true)
{
    /*
    // This version does not send the boolean value as a variant, and does not seem to turn on the adapter
    DBusMessage* dbus_msg = dbus_message_new_method_call("org.bluez", adapter_path, "org.freedesktop.DBus.Properties", "Set");
    dbus_message_append_args(dbus_msg,
        DBUS_TYPE_STRING, &adapter,
        DBUS_TYPE_STRING, &powered,
        DBUS_TYPE_BOOLEAN, &cpTrue,
        DBUS_TYPE_INVALID);
    dbus_connection_send(dbus_conn, dbus_msg, NULL);
    dbus_message_unref(dbus_msg);
    */
    // This version was hacked from looking at https://git.kernel.org/pub/scm/network/connman/connman.git/tree/gdbus/client.c#n667
    // https://www.mankier.com/5/org.bluez.Adapter#Interface-boolean_Powered_%5Breadwrite%5D
    DBusMessage* dbus_msg = dbus_message_new_method_call("org.bluez", adapter_path, "org.freedesktop.DBus.Properties", "Set"); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga98ddc82450d818138ef326a284201ee0
    if (!dbus_msg)
        std::cerr << "Can't allocate new method call" << std::endl;
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
        dbus_message_unref(dbus_msg); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gab69441efe683918f6a82469c8763f464
    }
}
// These three helped me figure out how to send an array of DICT entries
// https://stackoverflow.com/questions/29973486/d-bus-how-to-create-and-send-a-dict
// https://git.kernel.org/pub/scm/network/connman/connman.git/tree/gdbus/client.c#n667
// https://android.googlesource.com/platform/external/wpa_supplicant_8/+/master/wpa_supplicant/dbus/dbus_dict_helpers.c

void bluez_filter_le(DBusConnection* dbus_conn, const char* adapter_path = "/org/bluez/hci0", const bool DuplicateData = true)
{
    // https://www.mankier.com/5/org.bluez.Adapter#Interface-void_SetDiscoveryFilter(dict_filter)
    DBusMessage* dbus_msg = dbus_message_new_method_call("org.bluez", adapter_path, "org.bluez.Adapter1", "SetDiscoveryFilter");
    if (!dbus_msg)
        std::cerr << "Can't allocate new method call" << std::endl;
    else
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
        dbus_connection_send(dbus_conn, dbus_msg, NULL);
        dbus_message_unref(dbus_msg);
    }
}

void bluez_find_adapters(DBusConnection* dbus_conn, std::vector<std::string> &adapter_paths)
{
    // Initialize D-Bus error
    DBusError dbus_error;
    dbus_error_init(&dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusErrors.html#ga8937f0b7cdf8554fa6305158ce453fbe
    DBusMessage* dbus_msg = dbus_message_new_method_call("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
    if (!dbus_msg)
        std::cerr << "Can't allocate new method call" << std::endl;
    else
    {
        dbus_error_init(&dbus_error);
        DBusMessage* dbus_reply = dbus_connection_send_with_reply_and_block(dbus_conn, dbus_msg, -1, &dbus_error);
        if (!dbus_reply)
        {
            std::cerr << "Can't get bluez managed objects:" << std::endl;
            if (dbus_error_is_set(&dbus_error))
            {
                std::cerr << dbus_error.message << std::endl;
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
                std::cout << std::right << std::setw(20) << "Message Type: " << std::string(dbus_message_type_to_string(dbus_message_get_type(dbus_reply))) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaed63e4c2baaa50d782e8ebb7643def19
                const std::string dbus_reply_Signature(dbus_message_get_signature(dbus_reply));
                std::cout << std::right << std::setw(20) << "Signature: " << dbus_reply_Signature << std::endl;
                std::cout << std::right << std::setw(20) << "Destination: " << std::string(dbus_message_get_destination(dbus_reply)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaed63e4c2baaa50d782e8ebb7643def19
                std::cout << std::right << std::setw(20) << "Sender: " << std::string(dbus_message_get_sender(dbus_reply)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaed63e4c2baaa50d782e8ebb7643def19
                if (NULL != dbus_message_get_path(dbus_reply)) std::cout << std::right << std::setw(20) << "Path: " << std::string(dbus_message_get_path(dbus_reply)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga18adf731bb42d324fe2624407319e4af
                if (NULL != dbus_message_get_interface(dbus_reply)) std::cout << std::right << std::setw(20) << "Interface: " << std::string(dbus_message_get_interface(dbus_reply)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga1ad192bd4538cae556121a71b4e09d42
                if (NULL != dbus_message_get_member(dbus_reply)) std::cout << std::right << std::setw(20) << "Member: " << std::string(dbus_message_get_member(dbus_reply)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaf5c6b705c53db07a5ae2c6b76f230cf9
                if (NULL != dbus_message_get_container_instance(dbus_reply)) std::cout << std::right << std::setw(20) << "Container Instance: " << std::string(dbus_message_get_container_instance(dbus_reply)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaed63e4c2baaa50d782e8ebb7643def19
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
                                if (DBUS_TYPE_OBJECT_PATH == dbus_message_iter_get_arg_type(&dict1_iter))
                                {
                                    DBusBasicValue value;
                                    dbus_message_iter_get_basic(&dict1_iter, &value);
                                    dict1_object_path = std::string(value.str);
                                    std::cout << std::right << std::setw(24) << "Object Path: " << dict1_object_path << std::endl;
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
                                                std::cout << std::right << std::setw(24) << "String: " << val << std::endl;
                                                if (!val.compare("org.bluez.Adapter1"))
                                                    adapter_paths.push_back(dict1_object_path);
                                            }
                                        } while (dbus_message_iter_next(&dict2_iter));
                                    } while (dbus_message_iter_next(&array2_iter));
                                }
                                else
                                {
                                    std::string type_str;
                                    TypeToString(&dict1_iter, type_str);
                                    std::cout << std::right << std::setw(20) << "Unexpected type in message: " << type_str << std::endl;
                                }
                            } while (dbus_message_iter_next(&dict1_iter));
                        } while (dbus_message_iter_next(&array1_iter));
                    } while (dbus_message_iter_next(&root_iter));
                }
            }
            dbus_message_unref(dbus_reply);
        }
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
        std::cerr << "Error connecting to the D-Bus system bus: " << dbus_error.message << std::endl;
        dbus_error_free(&dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusErrors.html#gaac6c14ead14829ee4e090f39de6a7568
    }
    else
    {
        std::vector<std::string> BlueZAdapters;
        bluez_find_adapters(dbus_conn, BlueZAdapters);
        for (auto& adapter : BlueZAdapters)
            std::cout << "Bluetooth Adapter: " << adapter << std::endl;
        if (!BlueZAdapters.empty())
        {
            std::string BlueZAdapter(BlueZAdapters.front());

            bluez_power_on(dbus_conn, BlueZAdapter.c_str());
            bluez_filter_le(dbus_conn, BlueZAdapter.c_str());

            DBusMessage* dbus_msg = nullptr;

            // TODO Call Method StopDiscovery when done
            // https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/adapter-api.txt
            dbus_msg = dbus_message_new_method_call("org.bluez", BlueZAdapter.c_str(), "org.bluez.Adapter1", "StartDiscovery");
            if (!dbus_msg)
            {
                std::cerr << "Can't allocate new method call" << std::endl;
            }
            else
            {
                dbus_error_init(&dbus_error);
                DBusMessage* dbus_reply = dbus_connection_send_with_reply_and_block(dbus_conn, dbus_msg, -1, &dbus_error);
                dbus_message_unref(dbus_msg);
                if (!dbus_reply)
                {
                    std::cerr << "Can't call StartDiscovery:" << std::endl;
                    if (dbus_error_is_set(&dbus_error))
                    {
                        std::cerr << dbus_error.message << std::endl;
                        dbus_error_free(&dbus_error);
                    }
                }
                else
                {
                    dbus_message_unref(dbus_reply);
                    dbus_connection_flush(dbus_conn); // https://dbus.freedesktop.org/doc/api/html/group__DBusConnection.html#ga10e68d9d2f41d655a4151ddeb807ff54

                    //const char* match_rule = "type='signal',interface='org.freedesktop.DBus.ObjectManager',member='InterfacesAdded'";
                    const char* match_rule = "type='signal',member='InterfacesAdded'";
                    dbus_bus_add_match(dbus_conn, match_rule, &dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusBus.html#ga4eb6401ba014da3dbe3dc4e2a8e5b3ef
                    if (dbus_error_is_set(&dbus_error))
                    {
                        std::cerr << "Error adding a match rule on the D-Bus system bus: " << dbus_error.message << std::endl;
                        dbus_error_free(&dbus_error);
                    }
                    else
                    {
                        const char* match_rule2 = "type='signal',member='PropertiesChanged'";
                        dbus_bus_add_match(dbus_conn, match_rule2, &dbus_error); // https://dbus.freedesktop.org/doc/api/html/group__DBusBus.html#ga4eb6401ba014da3dbe3dc4e2a8e5b3ef
                        // Main loop
                        bool running = true;
                        while (running)
                        {
                            // Wait for access to the D-Bus
                            if (!dbus_connection_read_write(dbus_conn, 1000)) // https://dbus.freedesktop.org/doc/api/html/group__DBusConnection.html#ga371163b4955a6e0bf0f1f70f38390c14
                            {
                                ErrorLog("D-Bus connection was closed");
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
                                    std::cout << "Got message:\tType: " << dbus_msg_Type << "\tSignature: \"" << dbus_msg_Signature << "\"" << std::endl;
                                    if (DBUS_MESSAGE_TYPE_ERROR == dbus_message_get_type(dbus_msg))
                                        std::cout << "  Error name: " << std::string(dbus_message_get_error_name(dbus_msg)) << std::endl; // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga4e98b2283707a8e0313fc7c6fe3b1b5f
                                    else if (DBUS_MESSAGE_TYPE_SIGNAL == dbus_message_get_type(dbus_msg))
                                    {
                                        const std::string dbus_msg_Path(dbus_message_get_path(dbus_msg)); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga18adf731bb42d324fe2624407319e4af
                                        const std::string dbus_msg_Member(dbus_message_get_member(dbus_msg)); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gaf5c6b705c53db07a5ae2c6b76f230cf9
                                        const std::string dbus_msg_Interface(dbus_message_get_interface(dbus_msg)); // https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#ga1ad192bd4538cae556121a71b4e09d42
                                        //if (!dbus_msg_Path.compare("/org/bluez/hci0/dev_E3_8E_C8_C1_98_9A"))
                                        {
                                            std::cout << "  Path: " << dbus_msg_Path << std::endl;
                                            std::cout << "  Interface: " << dbus_msg_Interface << std::endl;
                                            std::cout << "  Member: " << dbus_msg_Member << std::endl;
                                            int indent = 4;
                                            // BLE advertisement messages arrive as InterfacesAdded messages
                                            if (!dbus_msg_Member.compare("InterfacesAdded"))
                                            {
                                                // Type: Signal
                                                // Path: /
                                                // Interface: org.freedesktop.DBus.ObjectManager
                                                // Member: InterfacesAdded
                                                // Signature: oa{sa{sv}}
                                                // o: DBUS_TYPE_OBJECT_PATH
                                                // a: DBUS_TYPE_ARRAY
                                                // {}: DBUS_DICT_ENTRY_BEGIN_CHAR + DBUS_DICT_ENTRY_END_CHAR
                                                // s: DBUS_TYPE_STRING
                                                // a: DBUS_TYPE_ARRAY
                                                // {}: DBUS_DICT_ENTRY_BEGIN_CHAR + DBUS_DICT_ENTRY_END_CHAR
                                                // s: DBUS_TYPE_STRING
                                                // v: DBUS_TYPE_VARIANT
                                                // Obj path value: /org/bluez/hci0/dev_F1_77_2B_1F_A3_FA

                                                HandleSignalMessage(dbus_msg, "/org/bluez/hci0/dev_", "org.bluez.Device1", indent);

                                                for (auto& iter : ble_properties)
                                                {
                                                    std::cout << __FILE__ << "(" << __LINE__ << "): " << iter.name << ": " << iter.value << std::endl;

                                                    // Clear value for next run
                                                    iter.value = "";
                                                }
                                            }
                                            else if (!dbus_msg_Member.compare("PropertiesChanged"))
                                            {
                                                if (!dbus_msg_Signature.compare("sa{sv}as"))
                                                {
                                                    const std::string dict_entry_filter("org.bluez.Device1");
                                                    DBusMessageIter root_iter;
                                                    dbus_message_iter_init(dbus_msg, &root_iter);
                                                    DBusBasicValue value;
                                                    dbus_message_iter_get_basic(&root_iter, &value);
                                                    const std::string dbus_msg_String(value.str);
                                                    std::cout << "  String: " << dbus_msg_String << std::endl;
                                                    if (!dbus_msg_String.compare(dict_entry_filter))
                                                    {
                                                        bool fail = false;
                                                        while (!fail && dbus_message_iter_next(&root_iter))
                                                        {
                                                            if (DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&root_iter))
                                                                HandleArray(&root_iter, dict_entry_filter, indent);
                                                            else
                                                            {
                                                                std::cout << "    Unexpected type in message: " << TypeToString(dbus_message_iter_get_arg_type(&root_iter)) << std::endl;
                                                                fail = true;
                                                            }
                                                        };
                                                        running = false;
                                                    }
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
                    }
                }
            }
            bluez_filter_le(dbus_conn, BlueZAdapter.c_str(), false);
        }
        // Close the connection
        //dbus_connection_close(dbus_conn);
        // When using the System Bus, unreference the connection instead of closing it
        dbus_connection_unref(dbus_conn);
    }
    return 0;
}
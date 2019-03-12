
#include <sqdb/sqdbuti.h>
#include <sqdb/sqdbmanager.h>
#include <sstream>
#include <codec/sqcodec.h>
#include <sqstd/sqstringhelper.h>
#include <google/protobuf/util/field_mask_util.h>


std::string FieldAttribute::m_key;

#ifndef MASK_MESSAGE_NAME
#define MASK_MESSAGE_NAME "google.protobuf.FieldMask"
#endif

SQDBUti::SQDBUti(void)
{
}


SQDBUti::~SQDBUti(void)
{
}

map<string, int> FieldAttribute::encrypt_field;
map<string, int> FieldAttribute::utf8_field;
string           FieldAttribute::aes_key;

FieldAttribute::fun_handle_ency FieldAttribute::m_fun_encry;
FieldAttribute::fun_handle_ency FieldAttribute::m_fun_decry;

FieldAttribute::tagFieldAttribute()
{	
	_type = 0;
	//option = false;
	_has_value = false;
	is_encrypt = false;
}

void FieldAttribute::name(const string& __name)
{
	_name = __name;
}
const string& FieldAttribute::name() const
{
	return _name;
}
bool FieldAttribute::has_value() const
{
	return _has_value;
}
int  FieldAttribute::type() const
{
	return _type;
}
string FieldAttribute::value_decrypt(const string& value,int type,bool has_value,bool encrypt)
{
	_value     = value;
	_type      = type;
	_has_value = has_value;
	string short_name;
	if(is_encrypt_field(_name, short_name))
	{
		_type = encrypt_field[short_name];
		if(encrypt && m_fun_decry)
		{
			_value = m_fun_decry(_value);
		}
	}
	return value;
}
string FieldAttribute::value(const string& value, int type, bool has_value)
{
	_value     = value;
	_type      = type;
	_has_value = has_value;
	string short_name;
	if(is_encrypt_field(_name, short_name))
	{
		_type = encrypt_field[short_name];
	}
	return value;
}
string FieldAttribute::value(int _value)
{	
	char sz_value[129] = {0}; 
	_i64toa_s(_value, sz_value, 128, 10);
	return value(sz_value, google::protobuf::FieldDescriptor::CppType::CPPTYPE_INT32);
}
string FieldAttribute::value(unsigned int _value)
{			
	return value((int)_value);
}
string FieldAttribute::value(__int64 _value)
{		
	char sz_value[129] = {0}; 
	_i64toa_s(_value, sz_value, 128, 10);
	return value(sz_value, google::protobuf::FieldDescriptor::CppType::CPPTYPE_INT64);
}
string FieldAttribute::value(unsigned __int64 _value)
{			
	return value((__int64)_value);
}
string FieldAttribute::value() const
{
	return _value;
}

const string& FieldAttribute::refvalue() const
{
    return _value;
}

string FieldAttribute::add_quto(const string& value)
{
	string sqlvalue = "'";
    sqlvalue.append(value);
    sqlvalue.append("'");
	return sqlvalue;
}
string FieldAttribute::sql_value() const
{	
	string sqlvalue = _value;	
	if(_type == protobuf::FieldDescriptor::CppType::CPPTYPE_STRING)
	{
		sqlvalue = "\'" + sqlvalue + "\'";
	}
	else if(_type != protobuf::FieldDescriptor::CppType::CPPTYPE_STRING)
	{
		if(sqlvalue.empty())
		{
			sqlvalue = "0";
		}
	}
	return sqlvalue;
}
void FieldAttribute::set_encrpyt_flag(bool flag)
{
	is_encrypt = flag;
}
string FieldAttribute::encrypt_sql_value() const
{
	string field_value = sql_value();
	string short_name;
	if(is_encrypt_field(_name, short_name))
	{
		field_value = encrypt_value(_value);
		const_cast<FieldAttribute *>(this)->set_encrpyt_flag(true);
	}

	return field_value;
}

string FieldAttribute::encrypt_valueex(const string& sql_value)
{
    if(m_fun_encry != NULL)
    {
        return m_fun_encry(sql_value);
    }

    return sql_value;
}

string FieldAttribute::encrypt_value(const string& sql_value)
{
	if(m_fun_encry != NULL)
	{
		return add_quto(m_fun_encry(sql_value));
	}
        
	return sql_value;
}

bool FieldAttribute::is_encrypt_field(const string& name, string& short_name)
{
	bool is_encrypt = false;
	short_name = name;
	string::size_type pos = short_name.find('.');
	if(pos != short_name.npos)
	{
		short_name.erase(0, pos + 1);
	}
	if(encrypt_field.count(short_name))
	{
		is_encrypt = true;
	}
	return is_encrypt;
}

bool FieldAttribute::is_utf8_field(const string& name, string& short_name)
{
    bool is_utf8 = false;
    short_name = name;
    string::size_type pos = short_name.find('.');
    if(pos != short_name.npos)
    {
        short_name.erase(0, pos + 1);
    }
    if(utf8_field.count(short_name))
    {
        is_utf8 = true;
    }
    return is_utf8;
}

int FieldAttribute::set_encrypt_cfg(const map<string, int>& _encrypt_field, const string& _aes_key)
{
	encrypt_field = _encrypt_field;
	aes_key = "\'" +_aes_key + "\'";
	return 0;
}

int FieldAttribute::set_encrypt_cfg(const map<string, int>& _encrypt_field, fun_handle_ency fun_ency, fun_handle_ency fun_decry)
{
	encrypt_field = _encrypt_field;

	m_fun_encry = fun_ency ;
	m_fun_decry = fun_decry;

	//m_key = inkey;
	return 0;
}

int FieldAttribute::set_utf8_cfg(const map<string/*字段名*/, int/*字段类型*/>& _utf8_field)
{
    utf8_field = _utf8_field;
    return 0;
}

const google::protobuf::Message *get_pbmask(const google::protobuf::Message& msg)
{
    const google::protobuf::Descriptor *d = msg.GetDescriptor();
    const google::protobuf::Reflection *ref = msg.GetReflection();
    const google::protobuf::Message *mask = nullptr;
    size_t count = d->field_count();

    for (size_t i = 0; i != count; ++i)
    {
        const google::protobuf::FieldDescriptor *field = d->field(i);
        if (field->name() != "mask") continue;
        if (field->cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) continue;
        if (field->message_type()->full_name() != MASK_MESSAGE_NAME) continue;

        mask = &((ref->GetMessage)(msg, field));
        break;
    }

    return mask;
}

bool has_mask_field(const google::protobuf::FieldDescriptor *field, const google::protobuf::Message *mask)
{
    if (nullptr == mask)
        return false;

    auto *p_mask = (google::protobuf::FieldMask*)(mask);
    return google::protobuf::util::FieldMaskUtil::IsPathInFieldMask(field->name(), *p_mask);
}

int SQDBUti::parse_message(const protobuf::Message& __in message, vector<FieldAttribute>& __out field_attributes, bool __in has_value, bool is_all)
{
	auto descriptor = message.GetDescriptor();
	auto reflection = message.GetReflection();
    auto pmask = (google::protobuf::FieldMask*)get_pbmask(message);
	for (auto i = 0; i < descriptor->field_count(); ++i)
	{
		auto field = descriptor->field(i);
		if(field == nullptr) continue;

		// 在插入记录的时候 即使 没有设置数据 也要插入默认数据，否则如果数据库设置了该字段不为null而且没有设置默认值，插入数据时忽略该字段将会报错 
		if(!reflection->HasField(message, field))
		{
            if (!has_value && field->name() != "mask")
            {
                FieldAttribute attribute;
                attribute.name(field->name());
                attribute.value("", field->cpp_type(), has_mask_field(field, pmask) || is_all);
                field_attributes.push_back(attribute);
                continue;
			}

            if (has_mask_field(field, pmask))
            {
                FieldAttribute attribute;
                attribute.name(field->name());
                attribute.value("", field->cpp_type(), true);
                field_attributes.push_back(attribute);
            }
			continue;
		}
		FieldAttribute attribute;			
		attribute.name(field->name());
		switch (field->cpp_type())
		{
		case protobuf::FieldDescriptor::CppType::CPPTYPE_INT32:      
			{	
				attribute.value(reflection->GetInt32(message, field));					
			}
			break; 
		case protobuf::FieldDescriptor::CppType::CPPTYPE_UINT32:      
			{	
				attribute.value(reflection->GetUInt32(message, field));					
			}
			break;
		case protobuf::FieldDescriptor::CppType::CPPTYPE_INT64:      
			{	
				attribute.value(reflection->GetInt64(message, field));					
			}
			break; 
		case protobuf::FieldDescriptor::CppType::CPPTYPE_UINT64:      
			{	
				attribute.value(reflection->GetUInt64(message, field));					
			}
			break; 
		case protobuf::FieldDescriptor::CppType::CPPTYPE_STRING:
			{						
				attribute.value(reflection->GetString(message, field));
			}  
			break;
        case protobuf::FieldDescriptor::CppType::CPPTYPE_MESSAGE:
            break;
		default:
			attribute.value("", field->cpp_type());
			fprintf(stderr, " parse_message do not data type, type:%d\n", field->cpp_type());
			break;
		}
		//attribute.has_value = reflection->HasField(message, field);
		field_attributes.push_back(attribute);
	}

	return 0;
}

int SQDBUti::parse_attribute(const vector<FieldAttribute>& __in field_attributes, protobuf::Message& __out message)
{
	auto descriptor = message.GetDescriptor();
	auto reflection = message.GetReflection();
	for(size_t i = 0; i < field_attributes.size(); ++i)
	{
		auto field = descriptor->FindFieldByName(field_attributes[i].name());
		if(field == nullptr)
		{
			continue;
		}
		switch (field->cpp_type())
		{
		case protobuf::FieldDescriptor::CppType::CPPTYPE_INT32:      
			{	
				reflection->SetInt32(&message, field, snqu::str::ToInt32(field_attributes[i].value()));
			}
			break; 
		case protobuf::FieldDescriptor::CppType::CPPTYPE_UINT32:
			{	
				reflection->SetUInt32(&message, field, snqu::str::ToInt32(field_attributes[i].refvalue()));
			}
			break; 
		case protobuf::FieldDescriptor::CppType::CPPTYPE_INT64:      
			{	
				reflection->SetInt64(&message, field, snqu::str::ToInt64(field_attributes[i].refvalue()));
			}
			break;  
		case protobuf::FieldDescriptor::CppType::CPPTYPE_UINT64:
			{	
				reflection->SetUInt64(&message, field, snqu::str::ToInt64(field_attributes[i].refvalue()));
			}
			break; 
		case protobuf::FieldDescriptor::CppType::CPPTYPE_STRING:
			{						
				reflection->SetString(&message, field, field_attributes[i].refvalue());
			}  
			break;
		default:
			fprintf(stderr, " parse_attribute do not data type, type:%d\n", field->cpp_type());
			break;
		}
	}
	return 0;
}

FieldAttribute* SQDBUti::find_field_attribute_by_name(vector<FieldAttribute>& __in field_attributes, const string& name)
{
	FieldAttribute* field = nullptr;
	for(size_t i = 0; i < field_attributes.size(); ++i)
	{
		if(field_attributes[i].name() == name)
		{
			field = &field_attributes[i];
			break;
		}
	}
	return field;
}

const FieldAttribute* SQDBUti::find_field_attribute_by_name(const vector<FieldAttribute>& __in field_attributes, const string& name)
{
	const FieldAttribute* field = nullptr;
	for(size_t i = 0; i < field_attributes.size(); ++i)
	{
		if(field_attributes[i].name() == name)
		{
			field = &field_attributes[i];
			break;
		}
	}
	return field;
}

int SQDBUti::set_encrypt_option(const map<string/*字段名*/, int/*字段类型*/>& encrypt_field, const string& aes_key)
{
	FieldAttribute::set_encrypt_cfg(encrypt_field, aes_key);
	return 0;
}

SqlStream& SqlStream::operator << (__int64 data)
{
    std::string short_name;
    if (is_value)
    {
        is_value = false;
        if(tagFieldAttribute::is_encrypt_field(last_item_name, short_name))
        {
            m_ss << tagFieldAttribute::encrypt_value(to_string(data));
        }
        else
            m_ss << data;
    }
    else
        m_ss << data;

    return *this;
}

SqlStream& SqlStream::operator << (int data)
{
    *this << __int64(data);
    return *this;
}

SqlStream& SqlStream::operator << (std::string data)
{
    if (is_value)
    {
        is_value = false;
        string short_name;
        if (tagFieldAttribute::is_encrypt_field(last_item_name, short_name))
        {
            m_ss << tagFieldAttribute::encrypt_value(data);
        }
        else if(tagFieldAttribute::is_utf8_field(last_item_name, short_name))
        {
            m_ss << "'" << snqu::codec::U2A(data) << "'";
        }
        else
            m_ss << "'" << data << "'";
    }
    else
    {
        last_item_name = data;
        m_ss << data;
    }
    return *this;
}

SqlStream& SqlStream::operator << (char* data)
{
    if (is_value)
    {
        is_value = false;
        string short_name;
        if (tagFieldAttribute::is_encrypt_field(last_item_name, short_name))
        {
            m_ss << tagFieldAttribute::encrypt_value(data);
        }
        else
            m_ss << "'" << data << "'";
    }
    else
    {
        last_item_name = data;
        m_ss << data;
    }
    return *this;
}

SqlStream& SqlStream::operator<< (const opertype& data)
{
    switch (data)
    {
    case ssbracketl:
        m_ss << '(';
        break;
    case ssbracketr:
        m_ss << ')';
        break;
    case ssisequal:
        m_ss << '=';
        is_value = true;
        break;
    case ssunequal:
        m_ss << "<>";
        is_value = true;
        break;
    case ssbigger:
        m_ss << '>';
        is_value = true;
        break;
    case sssmaller:
        m_ss << '<';
        is_value = true;
        break;
    case sswhere:
        m_ss << " where ";
        break;
    case ssand:
        m_ss << " and ";
        break;
	case ssset:
		m_ss<<" set ";
		break;
	case ssdot:
		m_ss<<",";
		break;
    default:
        break;
    }

    return *this;
}

SqlStream::SqlStream()
    : is_value(false)
{
}
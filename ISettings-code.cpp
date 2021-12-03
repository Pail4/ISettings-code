// ISettings-code.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <conio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

using namespace std;

enum DataType
{
	dtInteger,
	dtFloat,
	dtBoolean,
	dtString,
	dtUnknown
};


class ISettingsValue
{
public:
	virtual void SetValue(const string& value) = 0;
	virtual void SetValue(DataType type, const string& value) = 0;
	virtual string AsString() = 0;
	virtual int AsInteger() = 0;
	virtual double AsDouble() = 0;
	virtual bool AsBoolean() = 0;
	virtual DataType GetType() = 0;
	virtual ~ISettingsValue() {};
};

class ISettings
{
public:
	virtual bool LoadFromFile(const string& name) = 0;
	virtual bool SaveToFile(const string& name) = 0;
	virtual ISettingsValue& Get(const string& paramName) = 0;
	virtual int GetInteger(const string& paramName) = 0;
	virtual double GetFloat(const string& paramName) = 0;
	virtual bool GetBoolean(const string& paramName) = 0;
	virtual string GetString(const string& paramName) = 0;
	virtual void SetValue(const string& paramName, ISettingsValue& value) = 0;
	virtual void SetValue(const string& paramName, int value) = 0;
	virtual void SetValue(const string& paramName, double value) = 0;
	virtual void SetValue(const string& paramName, bool value) = 0;
	virtual void SetValue(const string& paramName, const char* value) = 0;
	virtual void SetInteger(const string& paramName, int value) = 0;
	virtual void SetFloat(const string& paramName, double value) = 0;
	virtual void SetBoolean(const string& paramName, bool value) = 0;
	virtual void SetString(const string& paramName, const string& value) = 0;
	virtual ~ISettings() {};
};



class SettingsValue : public ISettingsValue

{
	string value = "";
	DataType type = dtUnknown;

	bool isString(string str) //если есть что-то, кроме цифр !в том числе точка!
	{
		return true; //Пока не знаю других критериев, так что всё, что дойдет сюда(не пустое, не логическое,
					//не целое и не вещественное) будет строкой
	}

	bool isDigit(char chr)
	{
		return (chr >= '0' && chr <= '9');
	}

	bool isInteger(string str) // если есть только цифры
	{
		stringstream s(str);
		char chr;
		if (str[0] == '-')
			s.get(chr);
		while (s.get(chr))
		{
			if (!isDigit(chr))
				return false;
		}
		return true;
	}

	bool isFloat(string str) // Если это числа и есть 1 точка (кстати, такое - ".51" тоже прокатит = 0.51 - это фича)
	{
		stringstream s(str);
		char chr;
		bool flag = false;

		if (str[0] == '-')
			s.get(chr);
		while (s.get(chr))
		{
			if (!isDigit(chr) && (chr != '.' || flag))
				return false;
			else if (chr == '.')
				flag = true;
		}
		return true;
	}

public:

	virtual void SetValue(const string& value)
	{
		if (value.empty())
			SetValue(dtUnknown, value);

		else if (value == "true" || value == "false")
			SetValue(dtBoolean, value);

		else if (isInteger(value))
			SetValue(dtInteger, value);

		else if (isFloat(value))
			SetValue(dtFloat, value);

		else if (isString(value))
			SetValue(dtString, value);

		else
			SetValue(dtUnknown, value);
	}

	virtual void SetValue(DataType type, const string& value)
	{
		this->value = value;
		this->type = type;
	}

	virtual string AsString()
	{
		switch (GetType())
		{
		case 2:
			if (AsBoolean())
				return "true";
			return "false";
		default:
			return value;
		}
	}

	virtual int AsInteger()
	{
		stringstream valueStream(value);
		int result = 0;
		valueStream >> result;
		return result;
	}

	virtual double AsDouble()
	{
		stringstream valueStream(value);
		double result = 0;
		valueStream >> result;
		return result;
	}

	virtual bool AsBoolean()
	{
		if (value == "false" || value == "0" || value.empty())
			return false;
		return true;
	}

	virtual DataType GetType()
	{
		return type;
	}

	//virtual ~SettingsValue() {};
};

class Settings : public ISettings
{
	map <string, SettingsValue> list;

	bool isValidId(string str)
	{
		stringstream ss(str);
		char chr;
		if (str[0] >= '0' && str[0] <= '9')
			return false;
		while (ss.get(chr))
		{
			if (!(chr == '_' || chr >= 'a' && chr <= 'z' || chr >= 'A' && chr <= 'Z' || chr >= '0' && chr <= '9'))
			{
				return false;
			}
		}
		return true;
	}

	string cutString(string str)
	{
		return str.substr(1, str.size() - 2);
	}

	bool checkValueType(string str, DataType type)
	{
		SettingsValue test;
		test.SetValue(str);
		return (test.GetType() == type);
	}

public:

	virtual bool LoadFromFile(const string& name)
	{
		ifstream file(name);
		if (!file)
			return false;
		string line = "";
		while (getline(file, line))
		{
			stringstream ss(line);
			string paramName, sum, value;
			ss >> paramName >> sum >> value;

			if (sum != "=" || !isValidId(paramName))
				continue;
			if (value[0] == '\"')
			{
				while (value[(value.size() - 1)] != '\"')
				{
					string buf;
					ss >> buf;
					value += " " + buf;
				}
				value = cutString(value);
			}
			else if (checkValueType(value, dtString))
				continue;

			list[paramName].SetValue(value);
		}
		return true;
	}

	virtual bool SaveToFile(const string& name)
	{
		ofstream file(name);
		if (list.empty())
			return true;

		if (file)
		{
			for (auto item : list)
			{
				file << item.first << " = ";
				if (item.second.GetType() == 3)
					file << "\"" + item.second.AsString() + "\"" << "\n";
				else
					file << item.second.AsString() << "\n";
			}

			file.close();
			return true;
		}
		return false;
	}

	virtual ISettingsValue& Get (const string& paramName)
	{
		return list[paramName];
	}

	virtual int GetInteger(const string& paramName)
	{
		return list[paramName].AsInteger();
	}

	virtual double GetFloat(const string& paramName)
	{
		return list[paramName].AsDouble();
	}

	virtual bool GetBoolean(const string& paramName)
	{
		return list[paramName].AsBoolean();
	}

	virtual string GetString(const string& paramName)
	{
		return list[paramName].AsString();
	}

	virtual void SetValue(const string& paramName, ISettingsValue& value)
	{
		if (!isValidId(paramName))
			throw invalid_argument("invalid parametr name");
		list[paramName].SetValue(value.AsString());
	}

	virtual void SetValue(const string& paramName, int value)
	{
		if (!isValidId(paramName))
			throw invalid_argument("invalid parametr name");
		ostringstream oss;
		oss << value;
		list[paramName].SetValue(dtInteger, oss.str());
	}

	virtual void SetValue(const string& paramName, double value)
	{
		if (!isValidId(paramName))
			throw invalid_argument("invalid parametr name");
		ostringstream oss;
		oss << value;
		list[paramName].SetValue(dtFloat, oss.str());
	}

	virtual void SetValue(const string& paramName, bool value)
	{
		if (!isValidId(paramName))
			throw invalid_argument("invalid parametr name");
		ostringstream oss;
		oss << value;
		list[paramName].SetValue(dtBoolean, oss.str());
	}

	virtual void SetValue(const string& paramName, const char* value)
	{
		if (!isValidId(paramName))
			throw invalid_argument("invalid parametr name");
		ostringstream oss;
		oss << value;
		list[paramName].SetValue(dtString, oss.str());
	}

	virtual void SetInteger(const string& paramName, int value)
	{
		SetValue(paramName, value);
	}

	virtual void SetFloat(const string& paramName, double value)
	{
		SetValue(paramName, value);
	}

	virtual void SetBoolean(const string& paramName, bool value)
	{
		SetValue(paramName, value);
	}

	virtual void SetString(const string& paramName, const string& value)
	{
		if (!isValidId(paramName))
			throw invalid_argument("invalid parametr name");
		ostringstream oss;
		oss << value;
		list[paramName].SetValue(dtString, oss.str());
	}

	//virtual ~Settings(){};

};



int main()
{
	Settings set;
	//set.SetValue("*93284===", "string");

	set.LoadFromFile("settings.txt");

	set.SaveToFile("settings.txt");
}

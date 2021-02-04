#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>

const char* ERROR_OPEN_FILE = "File can't be open.\n";
const char* ERROR_FORMAT_SIZE = "Invalid input format. You need provide 15x15 table.\n";
const char* ERROR_FORMAT_DATA = "Invalid input format.Data must be equal '0' or '1'.\n";
const char* ERROR_FORMAT_FRAME = "Invalid input format. Correct 'zero'-frame of data.\n";

const std::string FILE_NAME = "testInput/input16.txt";
const int ARRAY_SIZE = 15;

const short LEFT_FRAME_ELEMENT  = 16384;
const short RIGHT_FRAME_ELEMENT = 1;

const short FIVE_BIT_SIDE = 31;
const short TEN_BIT_SIDE  = 1023;

const float AVAILABLE_DIFFERENCE_PERCENT = 0.2f;

struct Point
{
	short x, y;
};

struct Shape
{
	std::string name;
	Point point;
	short length;

	friend std::ostream& operator<<(std::ostream& out, const Shape& shape);
};

Shape __fastcall isSquare(const std::vector<short>& data);
Shape __fastcall isCircle(const std::vector<short>& data);

short __fastcall computeRepeatsInRow(short digit);
short __fastcall generateMask(short repeats);
short __fastcall countOne(short digit, short shift = 0);
bool  __fastcall checkDifference( short mask, short digit, short difference);
short __fastcall computeXCoordinate(short digit);

short __fastcall newLine(short digit);
short __fastcall computeDiameter(short diametr, short shift);

int main(int argc, char** argv)
{
	std::ifstream input(FILE_NAME);
	if (!input.is_open())
	{
		std::cerr << ERROR_OPEN_FILE;
		return 1;
	}

	std::vector<short> data;
	std::string row = "";

	while (!input.eof())
	{
		std::getline(input, row);
		if (row.length() != ARRAY_SIZE)
		{
			input.close();
			std::cerr << ERROR_FORMAT_SIZE;
			return 1;
		}
		short int number;
		try
		{
			size_t end;
			number = std::stoi(row, &end, 2);
			if (row[end] != '\0')
			{
				input.close();
				std::cerr << ERROR_FORMAT_DATA;
				return 1;
			}
			if ((number & LEFT_FRAME_ELEMENT) || (number & RIGHT_FRAME_ELEMENT))
			{
				input.close();
				std::cerr << ERROR_FORMAT_FRAME;
				return 1;
			}
		}
		catch (const std::invalid_argument& error)
		{
			input.close();
			std::cerr << ERROR_FORMAT_DATA;
			return 1;
		}
		catch (const std::out_of_range& error)
		{	
			input.close();
			std::cerr << ERROR_FORMAT_SIZE;
			return 1;
		}
		data.push_back(number);
	}
	input.close();
	if (data.size() != ARRAY_SIZE)
	{
		std::cerr << ERROR_FORMAT_SIZE;
		return 1;
	}
	if ((data.front() != 0) || (data.back() != 0))
	{
		std::cerr << ERROR_FORMAT_FRAME;
		return 1;
	}

	Shape result = isSquare(data);
	if (result.name == "Unknown figure")
	{
		result = isCircle(data);
	}
	std::cout << result;
	return 0;
}

Shape __fastcall isSquare(const std::vector<short>& data)
{
	short repeatsInRow = 0;
	short repeatingRows = 0;
	short digit = 0;
	short oldDigit = 0;
	short indexOfDigit = 0;
	short upper = 0;
	short lower = 0;
	short shiftCounter = 0;

	for (int i = 1; i < ARRAY_SIZE - 1; ++i)
	{
		if (digit == 0)
		{
			if (i != indexOfDigit)
			{
				shiftCounter = 0;
			}
			digit = data[i];
			if (i == indexOfDigit)
			{
				digit -= oldDigit;
			}
			if (digit <= 0)
			{
				digit = 0;
				oldDigit = 0;
				continue;
			}
			digit >>= shiftCounter;
			while ((FIVE_BIT_SIDE != (digit & FIVE_BIT_SIDE)) && digit)
			{
				digit >>= 1;
				++shiftCounter;
			}	
			if ((digit & FIVE_BIT_SIDE) >= FIVE_BIT_SIDE && (digit & TEN_BIT_SIDE) <= TEN_BIT_SIDE)
			{	
				repeatsInRow = computeRepeatsInRow(digit);
				if (repeatsInRow < 5 || repeatsInRow > 10)
				{
					oldDigit += (digit << shiftCounter);
					digit = 0;
					indexOfDigit = i;
					--i;
					continue;
				}
				digit &= generateMask(repeatsInRow);
				digit <<= shiftCounter;
				++repeatingRows;
				indexOfDigit = i;
				upper = i - 1;
				lower = i + 1;
			}
		}
		else
		{
			short difference = static_cast<short>(countOne(digit, shiftCounter) * AVAILABLE_DIFFERENCE_PERCENT);
			bool upperAccess = true;
			while ((upper > 0 && upperAccess )|| lower < ARRAY_SIZE - 1)
			{
				if (upper > 0 && upperAccess)
				{
					if (checkDifference(digit, data[upper], difference))
					{
						++repeatingRows;
						--upper;
					}
					else
					{
						++upper;
						upperAccess = false;
					}
				}
				if (lower < ARRAY_SIZE - 1)
				{
					if (checkDifference(digit, data[lower], difference))
					{
						++repeatingRows;
						++lower;
					}
					else
					{
						lower = ARRAY_SIZE - 1;
					}
				}
				
			}
			if (upper == 0)
			{
				++upper;
			}
			if ((repeatingRows == repeatsInRow) && (repeatingRows >= 5))
			{
				return { "Square", { computeXCoordinate(digit) + 1, upper + 1}, repeatingRows };
			}
			else
			{
				repeatingRows = 0;
				oldDigit += digit;
				digit = 0;
				i = indexOfDigit - 1;
			}
		}
		
	}
	return { "Unknown figure", {0, 0}, 0 };
}

Shape __fastcall isCircle(const std::vector<short>& data)
{
	short repeatsInRow = 0;
	short repeatingRows = 0;
	short digit = 0;
	short diameter = 0;
	short indexOfDigit = 0;
	short oldDigit = 0;
	short upper = 0;
	short lower = 0;
	short shiftCounter = 0;

	for (int i = 2; i < ARRAY_SIZE - 2; ++i)
	{
		if (diameter == 0)
		{
			if (i != indexOfDigit)
			{
				shiftCounter = 0;
			}
			diameter = data[i];
			if (i == indexOfDigit)
			{
				diameter -= oldDigit;
			}
			if (diameter <= 0)
			{
				diameter = 0;
				oldDigit = 0;
				continue;
			}
			diameter <<= shiftCounter;
			while ((FIVE_BIT_SIDE != (diameter & FIVE_BIT_SIDE)) && (diameter != 0))
			{
				diameter >>= 1;
				++shiftCounter;
			}
			if ((diameter & FIVE_BIT_SIDE) >= FIVE_BIT_SIDE && (diameter & FIVE_BIT_SIDE) <= TEN_BIT_SIDE)
			{
				repeatsInRow = computeRepeatsInRow(diameter);
				if (repeatsInRow < 5 || repeatsInRow > 10)
				{
					oldDigit += (diameter << shiftCounter);
					diameter = 0;
					indexOfDigit = i;
					--i;
					continue;
				}
				diameter &= generateMask(repeatsInRow);
				diameter <<= shiftCounter;
				indexOfDigit = i;
				upper = i - 1;
				lower = i + 1;
				repeatingRows = 1;
			}
		}
		else
		{
			short difference = 1;
			bool upperAccess = true;
			bool lowerAccess = true;
			while ((upper > 0 && upperAccess) || (lower < ARRAY_SIZE - 1 && lowerAccess))
			{
				if (upper > 0 && upperAccess)
				{
					if (checkDifference(diameter, data[upper], difference))
					{
						repeatingRows++;
						--upper;
					}
					else
					{

						++upper;
						upperAccess = false;
					}
				}
				if (lower < ARRAY_SIZE - 1 && lowerAccess)
				{
					if (checkDifference(diameter, data[lower], difference))
					{
						++repeatingRows;
						++lower;
					}
					else
					{
						--lower;
						lowerAccess = false;
					}
				}
			} 
			if (repeatingRows >= 2 && repeatingRows <= 4 && upper > 0 && lower < ARRAY_SIZE - 1)
			{
				digit = diameter;
				--upper;
				++lower;
				short int endLine;
				switch (repeatsInRow)
				{
				case 6:
					endLine = 2;
					break;
				case 8:
				case 10:
					endLine = 4;
					break;
				default:
					endLine = 3;
					break;
				}
				while (repeatsInRow > endLine)
				{
					digit = newLine(digit);
					repeatsInRow -= 2;
					if (checkDifference(digit, data[lower], difference) && checkDifference(digit, data[upper], difference))
					{
						repeatingRows += 2;
						upper--;
						lower++;
						if (upper == 0 || lower == 14)
						{
							repeatsInRow = 0;
							oldDigit += diameter;
							i = indexOfDigit - 1;
						}
					}
					else
					{
						repeatsInRow = 0;
					}
				}
				if (computeDiameter(diameter, shiftCounter) == repeatingRows)
				{
					return { "Circle", {computeXCoordinate(data[indexOfDigit]) + 1 + (computeDiameter(diameter, shiftCounter) / 2), 
						((upper + lower) / 2) + 1 }, computeDiameter(diameter, shiftCounter) };
				}
				else
				{
					diameter = 0;
				}
			}
			else
			{
				repeatsInRow = 0;
				oldDigit += diameter;
				diameter = 0;
				i = indexOfDigit - 1;
			}
		}
	}
	return { "Unknown figure", {0, 0}, 0 };
}

short __fastcall computeRepeatsInRow(short digit)
{
	short result = 0;
	while (digit & 1)
	{
		++result;
		digit >>= 1;
	}
	return result;
}

short __fastcall generateMask(short repeats)
{
	int mask = 31;
	repeats -= 5;
	while (repeats)
	{
		mask <<= 1;
		++mask;
		--repeats;
	}
	return mask;
}

short __fastcall countOne(short digit, short shift)
{
	short counter = 0;
	digit >>= shift;
	while (digit)
	{
		if ((digit & 1))
		{
			++counter;
		}
		digit >>= 1;
	}
	return counter;
}

bool __fastcall checkDifference(short mask, short digit, short difference)
{
	short frameDigit = digit & (~mask);
	if ((frameDigit & (mask << 1)) != 0)
	{
		--difference;
	}
	if ((frameDigit & (mask >> 1)) != 0)
	{
		--difference;
	}
	digit &= mask;
	digit ^= mask;
	return (difference  >= countOne(digit, 0));
}

short __fastcall computeXCoordinate(short digit)
{
	int result = 0;
	while ((digit & LEFT_FRAME_ELEMENT) != LEFT_FRAME_ELEMENT)
	{
		digit <<= 1;
		++result;
	}
	return result;
}

short __fastcall newLine(short digit)
{
	short shiftCounter = 0;
	while ((digit & 1) != 1)
	{
		digit >>= 1;
		++shiftCounter;
	}
	digit >>= 2;
	return (digit << (shiftCounter + 1));
}

short __fastcall computeDiameter(short diameter, short shift)
{
	return computeRepeatsInRow(diameter >> shift);
}

std::ostream& operator<<(std::ostream& out, const Shape& shape)
{
	out << shape.name;
	if (shape.name == "Square")
	{
		out << " side: " << shape.length << " left corner: (" << shape.point.x << ", "
			<< shape.point.y << ")\n";
	}
	if (shape.name == "Circle")
	{
		out << " center: (" << shape.point.x << ", " << shape.point.y
			<< ") diameter: " << shape.length << '\n';
	}
	return out;
}

#pragma once
#ifndef HEADER_H
#include "Header.h"
#endif

std::string remove_line_containing(std::string text, const std::string& substring) {
	std::istringstream stream(text);
	std::string line;
	std::string result;

	while (std::getline(stream, line)) {
		if (line.find(substring) == std::string::npos) {
			result += line + "\n";
		}
	}
	text = result;
	return text;
}

std::string extract_tag_bodies(const std::string& text, const std::string& regex) {
	std::regex pattern(regex);
	std::sregex_iterator next(text.begin(), text.end(), pattern);
	std::sregex_iterator end;
	std::string result;

	while (next != end) {
		std::smatch match = *next;
		result += match[1].str(); // ��������� ���������� ���� � ����������
		next++;
	}
	return result;
}

std::string remove_all_tags(const std::string& text, const std::string& regex) {
	// ���������� ��������� ��� ������ <tag>...</tag>
	std::regex pattern(regex);
	// ������ ���������� �� ������ ������
	return std::regex_replace(text, pattern, "");
}

std::string remove_all_words(std::string text, const std::string& word) {
	size_t pos = 0;
	// ���� ����� � ������
	while ((pos = text.find(word, pos)) != std::string::npos) {
		// ������� �����, ������� � ��������� �������
		text.erase(pos, word.length());
	}
	return text;
}

std::string sanitize_string(std::string output) {
	output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
	output.erase(std::remove(output.begin(), output.end(), '\t'), output.end());
	output.erase(std::remove(output.begin(), output.end(), '\r'), output.end());
	size_t pos = 0;
	while ((pos = output.find("  ", pos)) != std::string::npos) {
		output.replace(pos, 2, " ");
	}
	return output;
}

std::string replaceAll(std::string result, const std::string& from, const std::string& to) {
	size_t start_pos = 0;

	// ���� ������� ��������� ��������� `from`
	while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
		result.replace(start_pos, from.length(), to);
		start_pos += to.length(); // ���������� �������, ����� �������� ������������ �����
	}
	return result;
}
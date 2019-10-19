/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GraphicsDrawer.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggerardy <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/19 16:06:07 by ggerardy          #+#    #+#             */
/*   Updated: 2019/10/19 16:06:07 by ggerardy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <iostream>
#include <string>
#include <algorithm>
#include <limits>
#include <utility>
#include <iterator>
#include <locale>
#include <codecvt>
#include <cmath>

template <typename TKey, typename TVal>
class GraphicsDrawer {
public:
	template <typename TIter>
	GraphicsDrawer(const TIter& begin, const TIter& end)
	: draw_map_(height_ / braille_height_, std::vector<uint8_t>(width_ / braille_width_, 0x00))
	{
	  data_ = translate_data(begin, end);
	  set_min_max();
	  normalize_data();
	  fill_draw_map();
	}
	void draw() const {
	  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t, 0x10ffff, std::little_endian>> cvt;
	  //std::cout << cvt.to_bytes(unicode_top_arrow_) << std::endl;

	  for (auto it = draw_map_.rbegin(); it != draw_map_.rend(); ++ it) {
		//std::cout << cvt.to_bytes(unicode_vertical_pipe_) << ' ';
	    for (auto character : *it) {
	      wchar_t w_ch = braille_unicode_offset_ + character;
	      std::cout << cvt.to_bytes(w_ch);
	    }
	    std::cout << std::endl;
	  }
	}

private:
	void fill_draw_map() {
	  int i = 0;
	  for (auto value : data_) {
	    auto int_val = static_cast<int>(std::round(value));
		for (; int_val >= 0; --int_val) {
		  modify_char_for_var(int_val, i);
		}
	    ++i;
	  }
	}
	void modify_char_for_var(int val, int i) {
	  int x_char = i / braille_width_;
	  int x_dot  = i % braille_width_;
	  int y_char = val / braille_height_;
	  int y_dot  = val % braille_height_;

	  draw_map_[y_char][x_char] = modify_character(get_dot_idx(x_dot, y_dot), draw_map_[y_char][x_char]);
	}
	int get_dot_idx(int x_dot, int y_dot) {
	  y_dot = braille_height_ - 1 - y_dot;
	  if (y_dot < 3) {
		return y_dot + (x_dot != 0 ? 3 : 0);
	  } else {
		return x_dot ? 7 : 6;
	  }
	}
	uint8_t modify_character(int dot_idx, uint8_t ch) {
		ch |= braille_dots_switchers_[dot_idx];
	  return ch;
	}

	template <typename TIter>
	std::vector<double> translate_data(const TIter& begin, const TIter& end) const {
	  std::vector<double> translated;
	  translated.reserve(width_ / braille_width_);

	  size_t data_size = std::distance(begin, end);
	  int chunk_size = data_size <= width_ ? 1 : data_size / width_;

	  auto chunk_begin = begin;
	  while (chunk_begin != end) {
	    auto [new_key, new_val] = shrink_chunk(chunk_begin, safe_next(chunk_begin, end, chunk_size));
		chunk_begin = safe_next(chunk_begin, end, chunk_size);
		translated.push_back(static_cast<double>(new_val));
	  }

	  return translated;
	}
	void set_min_max() {
	  auto val_cmp_func = [](const auto& l, const auto& r) {return *l < *r;};
	  auto key_cmp_func = [](const auto& l, const auto& r) {return *l < *r;};

	  //min_key_ = find_min_max(data_.begin(), data_.end(), key_cmp_func)->first;
	  //max_key_ = find_min_max(data_.begin(), data_.end(), key_cmp_func, true)->first;
	  min_val_ = static_cast<TVal>(std::round(*find_min_max(data_.begin(), data_.end(), val_cmp_func)));
	  max_val_ = static_cast<TVal>(std::round(*find_min_max(data_.begin(), data_.end(), val_cmp_func, true)));

	  //std::cout << "Min key: " << min_key_ << std::endl;
	  //std::cout << "Max key: " << max_key_ << std::endl;
	  std::cout << "Min val: " << min_val_ << std::endl;
	  std::cout << "Max val: " << max_val_ << std::endl;
	}
	void normalize_data() {
	  TVal   max_normalized_val = max_val_ - min_val_;
	  double units_per_pixel    = static_cast<double>(max_normalized_val) / (height_ - braille_height_);
	  units_per_pixel += units_per_pixel == 0;

	  for (double& value : data_) {
		value = std::round((value - min_val_) / units_per_pixel);
	    value = value >= height_ ? height_ - 1 : value;
	  }
	}

	template <typename TIter, typename TFunc>
	static TIter find_min_max(const TIter& begin, const TIter& end, const TFunc& cmp_func, bool find_max = false) {
	  TIter res_iter = begin;

	  for (auto it = begin; it != end; ++it) {
	    if (!find_max) {
		  if (cmp_func(it, res_iter)) {
		    res_iter = it;
		  }
	    } else {
		  if (!cmp_func(it, res_iter)) {
			res_iter = it;
		  }
	    }
	  }
	  return res_iter;
	}
	template <typename TIter>
	static std::pair<double, double> shrink_chunk(const TIter& begin, const TIter& end) {
	  size_t count = 0;
	  double key_total = 0;
	  double val_total = 0;
	  for (auto it = begin; it != end; ++it) {
	    ++count;
	    key_total += it->first;
	    val_total += it->second;
	  }
	  return std::make_pair(key_total / count, val_total / count);
	}
	template <typename TIter>
	static TIter safe_next(const TIter& begin, const TIter& end, size_t count) {
	  size_t i = 0;
	  auto it = begin;
	  for (; it != end && i != count; ++it, ++i) {}
	  return it;
	}


	std::vector<double> data_;
	TKey min_key_ = 0;
	TKey max_key_ = 0;
	TVal min_val_ = 0;
	TVal max_val_ = 0;

	const int width_  = 100;
	const int height_ = 64;

	std::vector<std::vector<uint8_t>> draw_map_;

	static constexpr wchar_t braille_unicode_offset_ = L'\u2800';
	static constexpr wchar_t unicode_top_arrow_      = L'\u22c0';
	static constexpr wchar_t unicode_vertical_pipe_  = L'\u007c';
	static constexpr int braille_height_ = 4;
	static constexpr int braille_width_  = 2;
	static constexpr std::array<uint8_t, braille_height_ * braille_width_> braille_dots_switchers_ =
			{1, 2, 4, 8, 16, 32, 64, 128};
};

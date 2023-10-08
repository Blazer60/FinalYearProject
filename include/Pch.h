/**
 * @file Pch.h
 * @author Ryan Purse
 * @date 31/03/2022
 */


#pragma once

#ifdef PCH
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <algorithm>
#include <memory>
#include <numeric>
#include <cstdint>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <set>
#include <chrono>
#endif  // PCH

// pch.h is typically the first thing we include. glew.h must be included before gl.h, so we add it here to try and avoid errors.
#include <glew.h>

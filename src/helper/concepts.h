#pragma once

#include <concepts>


//-------------------------------------------------------------------------------------------------------------

// there's a chance this could be better but whatevs
template<class Container, class Value>
concept container_of = std::is_convertible_v<typename Container::value_type, Value>;

//-------------------------------------------------------------------------------------------------------------

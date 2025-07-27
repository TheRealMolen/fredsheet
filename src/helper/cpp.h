#pragma once


//-------------------------------------------------------------------------------------------------------------

#define NON_COPYABLE(typ)   typ(const typ&) = delete;       \
                            typ(typ&&) noexcept = delete;  \
                            typ& operator=(const typ&) = delete

#define NODISCARD   [[nodiscard]]

//-------------------------------------------------------------------------------------------------------------


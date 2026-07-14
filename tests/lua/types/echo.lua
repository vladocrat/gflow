---@meta echo

---@class (exact) Point
---@field x? integer
---@field y? integer

---@class (exact) EchoRequest
---@field message? string
---@field loud? boolean
---@field count? integer
---@field big? integer
---@field ratio? number
---@field blob? string
---@field color? Color
---@field origin? Point
---@field tags? string[]
---@field path? Point[]
---@field counters? table<string, integer>
---@field text_payload? string
---@field number_payload? integer

---@class (exact) EchoResponse
---@field message? string
---@field length? integer
---@field color? Color

---@class (exact) TickRequest
---@field ticks? integer
---@field interval_ms? integer

---@class (exact) Tick
---@field seq? integer
---@field label? string

local M = {}

---@param fields Point
---@return Point
function M.Point(fields) end

---@param fields EchoRequest
---@return EchoRequest
function M.EchoRequest(fields) end

---@param fields EchoResponse
---@return EchoResponse
function M.EchoResponse(fields) end

---@param fields TickRequest
---@return TickRequest
function M.TickRequest(fields) end

---@param fields Tick
---@return Tick
function M.Tick(fields) end

---@enum Color
M.Color = {
    COLOR_UNSPECIFIED = "COLOR_UNSPECIFIED",
    RED = "RED",
    GREEN = "GREEN",
    BLUE = "BLUE",
}

M.EchoService = {}

---@param req EchoRequest
---@return EchoResponse
function M.EchoService.Echo(req) end

---@param req TickRequest
---@return Tick
function M.EchoService.Subscribe(req) end

return M

#pragma once
#include "Drawing.hpp"
#include <Rendering/drawing.h>

static int DrawingObject_gc(lua_State* L);
static int DrawingObject_index(lua_State* L);
static int DrawingObject_newindex(lua_State* L);

static int SetupDrawingObjectMetatable(lua_State* L) {
	if (luaL_newmetatable(L, "DrawingObject") == 0) return 0;
	lua_pushcfunction(L, DrawingObject_index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, DrawingObject_newindex);
	lua_setfield(L, -2, "__newindex");
	lua_pushcfunction(L, DrawingObject_gc);
	lua_setfield(L, -2, "__gc");
	lua_pop(L, 1);
	return 1;
}

static ImColor NumberToColor(lua_Number num) {
	uint32_t c = static_cast<uint32_t>(num);
	float r = ((c >> 16) & 0xFF) / 255.0f;
	float g = ((c >> 8) & 0xFF) / 255.0f;
	float b = (c & 0xFF) / 255.0f;
	return ImColor(r, g, b);
}

static lua_Number ColorToNumber(const ImColor& color) {
	uint32_t r = static_cast<uint8_t>(color.Value.x * 255.0f);
	uint32_t g = static_cast<uint8_t>(color.Value.y * 255.0f);
	uint32_t b = static_cast<uint8_t>(color.Value.z * 255.0f);
	return static_cast<lua_Number>((r << 16) | (g << 8) | b);
}

static int PushVector2(lua_State* L, ImVec2 v) {
	lua_newtable(L);
	lua_pushnumber(L, v.x);
	lua_setfield(L, -2, "X");
	lua_pushnumber(L, v.y);
	lua_setfield(L, -2, "Y");
	return 1;
}

static ImVec2 CheckVector2(lua_State* L, int idx) {
	ImVec2 v{ 0, 0 };
	if (lua_istable(L, idx)) {
		lua_getfield(L, idx, "X");
		if (lua_isnumber(L, -1)) v.x = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, idx, "Y");
		if (lua_isnumber(L, -1)) v.y = lua_tonumber(L, -1);
		lua_pop(L, 1);
	} else if (lua_isnumber(L, idx)) {
		v.x = lua_tonumber(L, idx);
	}
	return v;
}

template<typename T>
static T* CheckDrawingObj(lua_State* L, int idx) {
	if (!lua_isuserdata(L, idx)) return nullptr;
	void* ud = lua_touserdata(L, idx);
	if (!ud) return nullptr;
	if (!lua_getmetatable(L, idx)) return nullptr;
	luaL_getmetatable(L, "DrawingObject");
	bool match = lua_rawequal(L, -1, -2);
	lua_pop(L, 2);
	if (!match) return nullptr;
	return *reinterpret_cast<T**>(ud);
}

static base_t* CheckDrawingBase(lua_State* L, int idx) {
	return CheckDrawingObj<base_t>(L, idx);
}

// ── Virtual method implementations ──

static int BaseIndex(base_t* obj, lua_State* L, const char* key) {
	if (strcmp(key, "Visible") == 0) { lua_pushboolean(L, obj->visible); return 1; }
	if (strcmp(key, "ZIndex") == 0) { lua_pushinteger(L, obj->z_index); return 1; }
	if (strcmp(key, "Transparency") == 0) { lua_pushnumber(L, obj->transparency); return 1; }
	if (strcmp(key, "Color") == 0) { lua_pushnumber(L, ColorToNumber(obj->color)); return 1; }
	if (strcmp(key, "Remove") == 0 || strcmp(key, "Destroy") == 0) {
		lua_pushcfunction(L, [](lua_State* L) -> int {
			base_t* obj = CheckDrawingBase(L, 1);
			if (!obj) return 0;
			auto it = std::find(drawing_cache.begin(), drawing_cache.end(), obj);
			if (it != drawing_cache.end()) drawing_cache.erase(it);
			key_map.erase(obj);
			void* ud = lua_touserdata(L, 1);
			if (ud) *reinterpret_cast<base_t**>(ud) = nullptr;
			delete obj;
			return 0;
		});
		return 1;
	}
	return 0;
}

static void BaseNewindex(base_t* obj, lua_State* L, const char* key) {
	if (strcmp(key, "Visible") == 0) { obj->visible = lua_toboolean(L, 3) != 0; return; }
	if (strcmp(key, "ZIndex") == 0) { obj->z_index = static_cast<int>(luaL_checkinteger(L, 3)); return; }
	if (strcmp(key, "Transparency") == 0) { obj->transparency = luaL_checknumber(L, 3); return; }
	if (strcmp(key, "Color") == 0) { obj->color = NumberToColor(luaL_checknumber(L, 3)); return; }
}

int base_t::__index(lua_State* L) {
	base_t* obj = CheckDrawingBase(L, 1);
	if (!obj) { lua_pushnil(L); return 1; }
	const char* key = luaL_checkstring(L, 2);
	int r = BaseIndex(obj, L, key);
	if (r) return r;
	lua_pushnil(L);
	return 1;
}

int base_t::__newindex(lua_State* L) {
	base_t* obj = CheckDrawingBase(L, 1);
	if (!obj) return 0;
	const char* key = luaL_checkstring(L, 2);
	BaseNewindex(obj, L, key);
	return 0;
}

void base_t::draw_obj() {}

// ── Line ──

int line_t::__index(lua_State* L) {
	line_t* obj = CheckDrawingObj<line_t>(L, 1);
	if (!obj) { lua_pushnil(L); return 1; }
	const char* key = luaL_checkstring(L, 2);
	int r = BaseIndex(obj, L, key);
	if (r) return r;
	if (strcmp(key, "From") == 0) return PushVector2(L, obj->from);
	if (strcmp(key, "To") == 0) return PushVector2(L, obj->to);
	if (strcmp(key, "Thickness") == 0) { lua_pushnumber(L, obj->thickness); return 1; }
	lua_pushnil(L);
	return 1;
}

int line_t::__newindex(lua_State* L) {
	line_t* obj = CheckDrawingObj<line_t>(L, 1);
	if (!obj) return 0;
	const char* key = luaL_checkstring(L, 2);
	BaseNewindex(obj, L, key);
	if (strcmp(key, "From") == 0) obj->from = CheckVector2(L, 3);
	else if (strcmp(key, "To") == 0) obj->to = CheckVector2(L, 3);
	else if (strcmp(key, "Thickness") == 0) obj->thickness = luaL_checknumber(L, 3);
	return 0;
}

void line_t::draw_obj() {}

// ── Text ──

int text_t::__index(lua_State* L) {
	text_t* obj = CheckDrawingObj<text_t>(L, 1);
	if (!obj) { lua_pushnil(L); return 1; }
	const char* key = luaL_checkstring(L, 2);
	int r = BaseIndex(obj, L, key);
	if (r) return r;
	if (strcmp(key, "Text") == 0) { lua_pushstring(L, obj->str.c_str()); return 1; }
	if (strcmp(key, "Size") == 0) { lua_pushnumber(L, obj->size); return 1; }
	if (strcmp(key, "Position") == 0) return PushVector2(L, obj->position);
	if (strcmp(key, "Center") == 0) { lua_pushboolean(L, obj->center); return 1; }
	if (strcmp(key, "Outline") == 0) { lua_pushboolean(L, obj->outline); return 1; }
	if (strcmp(key, "OutlineColor") == 0) { lua_pushnumber(L, ColorToNumber(obj->outline_color)); return 1; }
	if (strcmp(key, "OutlineTransparency") == 0) { lua_pushnumber(L, obj->outline_opacity); return 1; }
	if (strcmp(key, "Font") == 0) { lua_pushinteger(L, static_cast<int>(obj->font)); return 1; }
	if (strcmp(key, "TextBounds") == 0) return PushVector2(L, obj->text_bounds);
	lua_pushnil(L);
	return 1;
}

int text_t::__newindex(lua_State* L) {
	text_t* obj = CheckDrawingObj<text_t>(L, 1);
	if (!obj) return 0;
	const char* key = luaL_checkstring(L, 2);
	BaseNewindex(obj, L, key);
	if (strcmp(key, "Text") == 0) obj->str = luaL_checkstring(L, 3);
	else if (strcmp(key, "Size") == 0) obj->size = luaL_checknumber(L, 3);
	else if (strcmp(key, "Position") == 0) obj->position = CheckVector2(L, 3);
	else if (strcmp(key, "Center") == 0) obj->center = lua_toboolean(L, 3) != 0;
	else if (strcmp(key, "Outline") == 0) obj->outline = lua_toboolean(L, 3) != 0;
	else if (strcmp(key, "OutlineColor") == 0) obj->outline_color = NumberToColor(luaL_checknumber(L, 3));
	else if (strcmp(key, "OutlineTransparency") == 0) obj->outline_opacity = luaL_checknumber(L, 3);
	else if (strcmp(key, "Font") == 0) obj->font = static_cast<font_t>(luaL_checkinteger(L, 3));
	return 0;
}

void text_t::draw_obj() {}

// ── Image ──

int image_t::__index(lua_State* L) {
	image_t* obj = CheckDrawingObj<image_t>(L, 1);
	if (!obj) { lua_pushnil(L); return 1; }
	const char* key = luaL_checkstring(L, 2);
	int r = BaseIndex(obj, L, key);
	if (r) return r;
	if (strcmp(key, "Data") == 0) { lua_pushstring(L, obj->data.c_str()); return 1; }
	if (strcmp(key, "Size") == 0) return PushVector2(L, obj->size);
	if (strcmp(key, "Position") == 0) return PushVector2(L, obj->position);
	if (strcmp(key, "Rounding") == 0) { lua_pushnumber(L, obj->rounding); return 1; }
	lua_pushnil(L);
	return 1;
}

int image_t::__newindex(lua_State* L) {
	image_t* obj = CheckDrawingObj<image_t>(L, 1);
	if (!obj) return 0;
	const char* key = luaL_checkstring(L, 2);
	BaseNewindex(obj, L, key);
	if (strcmp(key, "Data") == 0) obj->data = luaL_checkstring(L, 3);
	else if (strcmp(key, "Size") == 0) obj->size = CheckVector2(L, 3);
	else if (strcmp(key, "Position") == 0) obj->position = CheckVector2(L, 3);
	else if (strcmp(key, "Rounding") == 0) obj->rounding = luaL_checknumber(L, 3);
	return 0;
}

void image_t::draw_obj() {}

// ── Circle ──

int circle_t::__index(lua_State* L) {
	circle_t* obj = CheckDrawingObj<circle_t>(L, 1);
	if (!obj) { lua_pushnil(L); return 1; }
	const char* key = luaL_checkstring(L, 2);
	int r = BaseIndex(obj, L, key);
	if (r) return r;
	if (strcmp(key, "Thickness") == 0) { lua_pushnumber(L, obj->thickness); return 1; }
	if (strcmp(key, "NumSides") == 0) { lua_pushnumber(L, obj->num_sides); return 1; }
	if (strcmp(key, "Radius") == 0) { lua_pushnumber(L, obj->radius); return 1; }
	if (strcmp(key, "Position") == 0) return PushVector2(L, obj->position);
	if (strcmp(key, "Filled") == 0) { lua_pushboolean(L, obj->filled); return 1; }
	lua_pushnil(L);
	return 1;
}

int circle_t::__newindex(lua_State* L) {
	circle_t* obj = CheckDrawingObj<circle_t>(L, 1);
	if (!obj) return 0;
	const char* key = luaL_checkstring(L, 2);
	BaseNewindex(obj, L, key);
	if (strcmp(key, "Thickness") == 0) obj->thickness = luaL_checknumber(L, 3);
	else if (strcmp(key, "NumSides") == 0) obj->num_sides = luaL_checknumber(L, 3);
	else if (strcmp(key, "Radius") == 0) obj->radius = luaL_checknumber(L, 3);
	else if (strcmp(key, "Position") == 0) obj->position = CheckVector2(L, 3);
	else if (strcmp(key, "Filled") == 0) obj->filled = lua_toboolean(L, 3) != 0;
	return 0;
}

void circle_t::draw_obj() {}

// ── Square ──

int square_t::__index(lua_State* L) {
	square_t* obj = CheckDrawingObj<square_t>(L, 1);
	if (!obj) { lua_pushnil(L); return 1; }
	const char* key = luaL_checkstring(L, 2);
	int r = BaseIndex(obj, L, key);
	if (r) return r;
	if (strcmp(key, "Thickness") == 0) { lua_pushnumber(L, obj->thickness); return 1; }
	if (strcmp(key, "Size") == 0) return PushVector2(L, obj->size);
	if (strcmp(key, "Position") == 0) return PushVector2(L, obj->position);
	if (strcmp(key, "Filled") == 0) { lua_pushboolean(L, obj->filled); return 1; }
	lua_pushnil(L);
	return 1;
}

int square_t::__newindex(lua_State* L) {
	square_t* obj = CheckDrawingObj<square_t>(L, 1);
	if (!obj) return 0;
	const char* key = luaL_checkstring(L, 2);
	BaseNewindex(obj, L, key);
	if (strcmp(key, "Thickness") == 0) obj->thickness = luaL_checknumber(L, 3);
	else if (strcmp(key, "Size") == 0) obj->size = CheckVector2(L, 3);
	else if (strcmp(key, "Position") == 0) obj->position = CheckVector2(L, 3);
	else if (strcmp(key, "Filled") == 0) obj->filled = lua_toboolean(L, 3) != 0;
	return 0;
}

void square_t::draw_obj() {}

// ── Quad ──

int quad_t::__index(lua_State* L) {
	quad_t* obj = CheckDrawingObj<quad_t>(L, 1);
	if (!obj) { lua_pushnil(L); return 1; }
	const char* key = luaL_checkstring(L, 2);
	int r = BaseIndex(obj, L, key);
	if (r) return r;
	if (strcmp(key, "Thickness") == 0) { lua_pushnumber(L, obj->thickness); return 1; }
	if (strcmp(key, "PointA") == 0) return PushVector2(L, obj->point_a);
	if (strcmp(key, "PointB") == 0) return PushVector2(L, obj->point_b);
	if (strcmp(key, "PointC") == 0) return PushVector2(L, obj->point_c);
	if (strcmp(key, "PointD") == 0) return PushVector2(L, obj->point_d);
	if (strcmp(key, "Filled") == 0) { lua_pushboolean(L, obj->filled); return 1; }
	lua_pushnil(L);
	return 1;
}

int quad_t::__newindex(lua_State* L) {
	quad_t* obj = CheckDrawingObj<quad_t>(L, 1);
	if (!obj) return 0;
	const char* key = luaL_checkstring(L, 2);
	BaseNewindex(obj, L, key);
	if (strcmp(key, "Thickness") == 0) obj->thickness = luaL_checknumber(L, 3);
	else if (strcmp(key, "PointA") == 0) obj->point_a = CheckVector2(L, 3);
	else if (strcmp(key, "PointB") == 0) obj->point_b = CheckVector2(L, 3);
	else if (strcmp(key, "PointC") == 0) obj->point_c = CheckVector2(L, 3);
	else if (strcmp(key, "PointD") == 0) obj->point_d = CheckVector2(L, 3);
	else if (strcmp(key, "Filled") == 0) obj->filled = lua_toboolean(L, 3) != 0;
	return 0;
}

void quad_t::draw_obj() {}

// ── Triangle ──

int triangle_t::__index(lua_State* L) {
	triangle_t* obj = CheckDrawingObj<triangle_t>(L, 1);
	if (!obj) { lua_pushnil(L); return 1; }
	const char* key = luaL_checkstring(L, 2);
	int r = BaseIndex(obj, L, key);
	if (r) return r;
	if (strcmp(key, "Thickness") == 0) { lua_pushnumber(L, obj->thickness); return 1; }
	if (strcmp(key, "PointA") == 0) return PushVector2(L, obj->point_a);
	if (strcmp(key, "PointB") == 0) return PushVector2(L, obj->point_b);
	if (strcmp(key, "PointC") == 0) return PushVector2(L, obj->point_c);
	if (strcmp(key, "Filled") == 0) { lua_pushboolean(L, obj->filled); return 1; }
	lua_pushnil(L);
	return 1;
}

int triangle_t::__newindex(lua_State* L) {
	triangle_t* obj = CheckDrawingObj<triangle_t>(L, 1);
	if (!obj) return 0;
	const char* key = luaL_checkstring(L, 2);
	BaseNewindex(obj, L, key);
	if (strcmp(key, "Thickness") == 0) obj->thickness = luaL_checknumber(L, 3);
	else if (strcmp(key, "PointA") == 0) obj->point_a = CheckVector2(L, 3);
	else if (strcmp(key, "PointB") == 0) obj->point_b = CheckVector2(L, 3);
	else if (strcmp(key, "PointC") == 0) obj->point_c = CheckVector2(L, 3);
	else if (strcmp(key, "Filled") == 0) obj->filled = lua_toboolean(L, 3) != 0;
	return 0;
}

void triangle_t::draw_obj() {}

// ── GC ──

static int DrawingObject_gc(lua_State* L) {
	base_t** ud = reinterpret_cast<base_t**>(lua_touserdata(L, 1));
	if (ud && *ud) {
		auto it = std::find(drawing_cache.begin(), drawing_cache.end(), *ud);
		if (it != drawing_cache.end()) drawing_cache.erase(it);
		key_map.erase(*ud);
		delete *ud;
		*ud = nullptr;
	}
	return 0;
}

// ── Metatable dispatch ──

static int DrawingObject_index(lua_State* L) {
	base_t* obj = CheckDrawingBase(L, 1);
	if (!obj) { lua_pushnil(L); return 1; }
	return obj->__index(L);
}

static int DrawingObject_newindex(lua_State* L) {
	base_t* obj = CheckDrawingBase(L, 1);
	if (!obj) return 0;
	return obj->__newindex(L);
}

// ── Module functions ──

static int Drawing_new(lua_State* L) {
	const char* type = luaL_checkstring(L, 1);
	SetupDrawingObjectMetatable(L);

	base_t* obj = nullptr;
	if (_stricmp(type, "Line") == 0) obj = new line_t();
	else if (_stricmp(type, "Text") == 0) obj = new text_t();
	else if (_stricmp(type, "Image") == 0) obj = new image_t();
	else if (_stricmp(type, "Circle") == 0) obj = new circle_t();
	else if (_stricmp(type, "Square") == 0) obj = new square_t();
	else if (_stricmp(type, "Quad") == 0) obj = new quad_t();
	else if (_stricmp(type, "Triangle") == 0) obj = new triangle_t();
	else return luaL_error(L, "Invalid Drawing type: '%s'", type);

	base_t** ud = static_cast<base_t**>(lua_newuserdata(L, sizeof(base_t*)));
	*ud = obj;
	luaL_getmetatable(L, "DrawingObject");
	lua_setmetatable(L, -2);

	drawing_cache.push_back(obj);
	key_map[obj] = static_cast<int>(drawing_cache.size());
	return 1;
}

static int Drawing_isrenderobj(lua_State* L) {
	lua_pushboolean(L, CheckDrawingBase(L, 1) != nullptr);
	return 1;
}

static int Drawing_cleardrawcache(lua_State* L) {
	for (auto obj : drawing_cache) delete obj;
	drawing_cache.clear();
	key_map.clear();
	return 0;
}

static int Drawing_getrenderproperty(lua_State* L) {
	base_t* obj = CheckDrawingBase(L, 1);
	if (!obj) { lua_pushnil(L); return 1; }
	const char* key = luaL_checkstring(L, 2);
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 2);
	return obj->__index(L);
}

static int Drawing_setrenderproperty(lua_State* L) {
	base_t* obj = CheckDrawingBase(L, 1);
	if (!obj) return 0;
	const char* key = luaL_checkstring(L, 2);
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 2);
	lua_pushvalue(L, 3);
	return obj->__newindex(L);
}

void CDrawing::InitLib(lua_State* L) {
	SetupDrawingObjectMetatable(L);

	// Drawing table
	lua_newtable(L);

	lua_pushcfunction(L, Drawing_new);
	lua_setfield(L, -2, "new");

	lua_newtable(L);
	lua_pushinteger(L, 0);
	lua_setfield(L, -2, "UI");
	lua_pushinteger(L, 1);
	lua_setfield(L, -2, "System");
	lua_pushinteger(L, 2);
	lua_setfield(L, -2, "Plex");
	lua_pushinteger(L, 3);
	lua_setfield(L, -2, "MonoSpace");
	lua_setfield(L, -2, "Fonts");

	lua_setglobal(L, "Drawing");

	// Global helper functions
	declare__Global(L, "isrenderobj", Drawing_isrenderobj);
	declare__Global(L, "cleardrawcache", Drawing_cleardrawcache);
	declare__Global(L, "getrenderproperty", Drawing_getrenderproperty);
	declare__Global(L, "setrenderproperty", Drawing_setrenderproperty);
}

#pragma once

#include <string>
#include <memory>
#include <type_traits>

#include "bakkesmodsdk/include/bakkesmod/wrappers/cvarmanagerwrapper.h"

template <class T>
class CVar2WayBinding
{
private:
	std::string cvarName;
	std::string description;
	bool searchable = true;
	bool hasMin = false;
	float min = 0;
	bool hasMax = false;
	float max = 0;
	bool saveToCfg = true;
	std::shared_ptr<T> ptr;
	std::unique_ptr<CVarWrapper> cvar;

public:
	CVar2WayBinding(
		std::string cvar,
		T defaultValue,
		std::string description = "",
		bool searchable = true,
		bool hasMin = false,
		float min = 0,
		bool hasMax = false,
		float max = 0,
		bool saveToCfg = true
	) : cvarName(cvar), description(description), searchable(searchable), hasMin(hasMin), min{min}, hasMax(hasMax), max{max}, saveToCfg(saveToCfg) {
		this->ptr = std::make_shared<T>(defaultValue);
	}

	CVarWrapper Register(const std::shared_ptr<CVarManagerWrapper>& cvarManager)
	{
		this->cvar = std::make_unique<CVarWrapper>(cvarManager->registerCvar(this->cvarName, "", description, searchable, hasMin, min, hasMax, max, saveToCfg));
		this->cvar->setValue(*this->ptr);
		this->cvar->bindTo(this->ptr);
		return *this->cvar;
	}

	CVarWrapper GetWrapper() const {
		return *this->cvar;
	}

	template <class T2 = T>
	void operator=(T val)
	{
		*this->ptr = val;
		this->cvar->setValue(val);
	}

    template <class T2 = T, std::enable_if_t<!std::disjunction_v<std::is_array<T2>, std::is_void<T2>>, int> = 0>
    _NODISCARD T2& operator*() const noexcept {
        return *Get();
    }

    template <class T2 = T, std::enable_if_t<!std::is_array_v<T2>, int> = 0>
    _NODISCARD T2* operator->() const noexcept {
        return Get();
    }

private:
	T* Get() const noexcept
	{
		return this->ptr.get();
	}
};


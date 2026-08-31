#pragma once

namespace RE
{
	template <class Key, class Value>
	class hkMapBase
	{
	public:
		struct Pair
		{
			Key key;
			Value value;
		};

		Pair* _data;            // 00
		int32_t _sizeAndFlags;  // 08
		int32_t _hashMod;       // 0C
	};

	template <class Key, class Value>
	class hkMap : public hkMapBase<Key, Value>
	{
	public:
	};

	template <class Key, class Value, class Allocator = void>
	class hkPointerMap
	{
	protected:
		hkMap<Key, Value> _map;  // 00
	};
	static_assert(sizeof(hkPointerMap<void*, void*>) == 0x10);

	class hkbStringEventPayload : public hkbEventPayload
	{
	public:
		inline static constexpr auto RTTI = RTTI_hkbStringEventPayload;
		inline static constexpr auto VTABLE = VTABLE_hkbStringEventPayload;

		// members
		hkStringPtr data;  // 10
	};
	static_assert(sizeof(hkbStringEventPayload) == 0x18);

	class hkbAnimationBindingWithTriggers : public hkReferencedObject
	{
	public:
		inline static constexpr auto RTTI = RTTI_hkbAnimationBindingWithTriggers;
		inline static constexpr auto VTABLE = VTABLE_hkbAnimationBindingWithTriggers;

		struct Trigger
		{
			Trigger(float a_time, int32_t a_eventId, hkbEventPayload* a_payload) :
				time(a_time),
				eventId(a_eventId),
				payload(a_payload)
			{}

			float time = 0.f;                             // 00
			uint32_t pad04 = 0;                           // 04
			int32_t eventId = -1;                         // 08
			hkRefPtr<hkbEventPayload> payload = nullptr;  // 10
		};
		static_assert(offsetof(Trigger, eventId) == 0x8);
		static_assert(sizeof(Trigger) == 0x18);

		// members
		hkRefPtr<hkaAnimationBinding> binding;  // 10
		hkArray<Trigger> triggers;              // 18
	};
	static_assert(sizeof(hkbAnimationBindingWithTriggers) == 0x28);

	class hkbSymbolIdMap : public hkReferencedObject
	{
	public:
		inline static constexpr auto RTTI = RTTI_hkbSymbolIdMap;
		inline static constexpr auto VTABLE = VTABLE_hkbSymbolIdMap;

		hkArray<int32_t> internalToExternalMap;
		hkPointerMap<int32_t, int32_t> externalToInternalMap;
	};

	namespace BSSynchronizedClipGeneratorUtils
	{
		class FindEventFunctor
		{
		public:
			inline static constexpr auto RTTI = RTTI_BSSynchronizedClipGeneratorUtils__FindEventFunctor;
			inline static constexpr auto VTABLE = VTABLE_BSSynchronizedClipGeneratorUtils__FindEventFunctor;

			virtual ~FindEventFunctor() = default;  // 00

			virtual void Unk_01([[maybe_unused]] const char* a_key) {}  // 01
		};
		static_assert(sizeof(FindEventFunctor) == 0x8);

		class BSHashMapEventFindFunctor : public FindEventFunctor
		{
		public:
			inline static constexpr auto RTTI = RTTI___BSHashMapEventFindFunctor;
			inline static constexpr auto VTABLE = VTABLE___BSHashMapEventFindFunctor;

			BSHashMapEventFindFunctor() { stl::emplace_vtable(this); }

			~BSHashMapEventFindFunctor() override {}  // 00

			// override (FindEventFunctor)
			void Unk_01([[maybe_unused]] const char* a_key) override {}  // 01

			// members
			void* map{ nullptr };  // 08
		};
		static_assert(sizeof(BSHashMapEventFindFunctor) == 0x10);
	}
}

inline void* hkHeapAlloc(std::int32_t a_numBytes)
{
	return RE::hkMemoryRouter::GetInstancePtr()->Heap->BlockAlloc(a_numBytes);
}

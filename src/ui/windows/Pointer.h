#pragma once

OMP_UI_WINDOWS_NAMESPACE_BEGIN

template <typename T>
class RefCounted : public IUnknown {
	volatile long m_nRefCount = 0;
	RefCounted(RefCounted&) = delete;
	RefCounted(RefCounted&&) = delete;
public:
	RefCounted() = default;
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) { return E_NOINTERFACE; }
	ULONG STDMETHODCALLTYPE AddRef(void) { return InterlockedIncrement(&m_nRefCount); }
	ULONG STDMETHODCALLTYPE Release(void) {
		ULONG nRefCount = InterlockedDecrement(&m_nRefCount);
		if (0 == nRefCount) delete static_cast<T*>(this);
		return nRefCount;
	}
};

template <typename T>
class WeakOwner : public RefCounted<WeakOwner<T>> {
	template <typename T>
	friend class WeakPtr;
	T* m_pObject;
public:
	WeakOwner(T* pObject) : m_pObject(pObject) {}
	~WeakOwner() { m_pObject = nullptr; }

	T* Get() { return m_pObject; }
	void Clear() { m_pObject = nullptr; }
};

template <typename T>
class SupportWeak : public RefCounted<T> {
	template <typename T>
	friend class WeakPtr;
	CComPtr<WeakOwner<T>> m_spWeakOwner = new WeakOwner<T>(static_cast<T*>(this));
public:
	SupportWeak() = default;
	~SupportWeak() { m_spWeakOwner->Clear(); }
};

template <typename T>
class WeakPtr {
	CComPtr<WeakOwner<T>> m_spWeakOwner;
public:
	WeakPtr() : m_spWeakOwner(new WeakOwner<T>(nullptr)) {}
	WeakPtr(SupportWeak<T>* pObject) : m_spWeakOwner(pObject->m_spWeakOwner) {}

	T* operator -> () const { return m_spWeakOwner ? m_spWeakOwner->m_pObject : nullptr; }
	operator CComPtr<T>() { return operator->(); }
	operator T* () { return operator->(); }
	operator bool() { return operator->(); }

	bool operator == (std::nullptr_t) { return operator->() == nullptr; }
	bool operator == (const T* pObject) { return operator->() == pObject; }
	bool operator !() { return !(operator->()); }

	friend bool operator == (std::nullptr_t, WeakPtr<T> pObject);
	friend bool operator == (T* pOther, WeakPtr<T> pObject);
};

template <typename T>
inline bool operator == (std::nullptr_t, WeakPtr<T> pObject) { return pObject == nullptr; }

template <typename T>
inline bool operator == (T* pOther, WeakPtr<T> pObject) { return pObject == pOther; }

OMP_UI_WINDOWS_NAMESPACE_END

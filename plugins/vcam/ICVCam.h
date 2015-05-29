
DECLARE_INTERFACE_(ICVCam, IUnknown)
{
    STDMETHOD(GetSize)(THIS_ LONG* width, LONG* height) PURE;
};

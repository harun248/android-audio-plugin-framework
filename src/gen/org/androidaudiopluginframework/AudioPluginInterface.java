/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package org.androidaudiopluginframework;
public interface AudioPluginInterface extends android.os.IInterface
{
  /** Default implementation for AudioPluginInterface. */
  public static class Default implements org.androidaudiopluginframework.AudioPluginInterface
  {
    @Override public void create(java.lang.String pluginId, int sampleRate) throws android.os.RemoteException
    {
    }
    @Override public boolean isPluginAlive() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void prepare(int frameCount, int bufferCount, long[] bufferPointers) throws android.os.RemoteException
    {
    }
    @Override public void activate() throws android.os.RemoteException
    {
    }
    @Override public void process(int timeoutInNanoseconds) throws android.os.RemoteException
    {
    }
    @Override public void deactivate() throws android.os.RemoteException
    {
    }
    @Override public int getStateSize() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void getState(long pointer) throws android.os.RemoteException
    {
    }
    @Override public void setState(long pointer, int size) throws android.os.RemoteException
    {
    }
    @Override public void destroy() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements org.androidaudiopluginframework.AudioPluginInterface
  {
    private static final java.lang.String DESCRIPTOR = "org.androidaudiopluginframework.AudioPluginInterface";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an org.androidaudiopluginframework.AudioPluginInterface interface,
     * generating a proxy if needed.
     */
    public static org.androidaudiopluginframework.AudioPluginInterface asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof org.androidaudiopluginframework.AudioPluginInterface))) {
        return ((org.androidaudiopluginframework.AudioPluginInterface)iin);
      }
      return new org.androidaudiopluginframework.AudioPluginInterface.Stub.Proxy(obj);
    }
    @Override public android.os.IBinder asBinder()
    {
      return this;
    }
    @Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
    {
      java.lang.String descriptor = DESCRIPTOR;
      switch (code)
      {
        case INTERFACE_TRANSACTION:
        {
          reply.writeString(descriptor);
          return true;
        }
        case TRANSACTION_create:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.create(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isPluginAlive:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isPluginAlive();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_prepare:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          long[] _arg2;
          _arg2 = data.createLongArray();
          this.prepare(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_activate:
        {
          data.enforceInterface(descriptor);
          this.activate();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_process:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.process(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_deactivate:
        {
          data.enforceInterface(descriptor);
          this.deactivate();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getStateSize:
        {
          data.enforceInterface(descriptor);
          int _result = this.getStateSize();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getState:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.getState(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setState:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          this.setState(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_destroy:
        {
          data.enforceInterface(descriptor);
          this.destroy();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements org.androidaudiopluginframework.AudioPluginInterface
    {
      private android.os.IBinder mRemote;
      Proxy(android.os.IBinder remote)
      {
        mRemote = remote;
      }
      @Override public android.os.IBinder asBinder()
      {
        return mRemote;
      }
      public java.lang.String getInterfaceDescriptor()
      {
        return DESCRIPTOR;
      }
      @Override public void create(java.lang.String pluginId, int sampleRate) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pluginId);
          _data.writeInt(sampleRate);
          boolean _status = mRemote.transact(Stub.TRANSACTION_create, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().create(pluginId, sampleRate);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isPluginAlive() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isPluginAlive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isPluginAlive();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void prepare(int frameCount, int bufferCount, long[] bufferPointers) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(frameCount);
          _data.writeInt(bufferCount);
          _data.writeLongArray(bufferPointers);
          boolean _status = mRemote.transact(Stub.TRANSACTION_prepare, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().prepare(frameCount, bufferCount, bufferPointers);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void activate() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_activate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().activate();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void process(int timeoutInNanoseconds) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(timeoutInNanoseconds);
          boolean _status = mRemote.transact(Stub.TRANSACTION_process, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().process(timeoutInNanoseconds);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void deactivate() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deactivate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deactivate();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getStateSize() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStateSize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getStateSize();
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void getState(long pointer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(pointer);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getState(pointer);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setState(long pointer, int size) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(pointer);
          _data.writeInt(size);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setState(pointer, size);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void destroy() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_destroy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().destroy();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static org.androidaudiopluginframework.AudioPluginInterface sDefaultImpl;
    }
    static final int TRANSACTION_create = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_isPluginAlive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_prepare = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_activate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_process = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_deactivate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getStateSize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_destroy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(org.androidaudiopluginframework.AudioPluginInterface impl) {
      if (Stub.Proxy.sDefaultImpl == null && impl != null) {
        Stub.Proxy.sDefaultImpl = impl;
        return true;
      }
      return false;
    }
    public static org.androidaudiopluginframework.AudioPluginInterface getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void create(java.lang.String pluginId, int sampleRate) throws android.os.RemoteException;
  public boolean isPluginAlive() throws android.os.RemoteException;
  public void prepare(int frameCount, int bufferCount, long[] bufferPointers) throws android.os.RemoteException;
  public void activate() throws android.os.RemoteException;
  public void process(int timeoutInNanoseconds) throws android.os.RemoteException;
  public void deactivate() throws android.os.RemoteException;
  public int getStateSize() throws android.os.RemoteException;
  public void getState(long pointer) throws android.os.RemoteException;
  public void setState(long pointer, int size) throws android.os.RemoteException;
  public void destroy() throws android.os.RemoteException;
}
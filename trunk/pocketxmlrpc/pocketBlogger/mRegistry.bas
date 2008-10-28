Attribute VB_Name = "mRegistry"
Option Explicit

Public Const NO_ERROR = 0                           ' Function returned successfully.
Public Const ERROR_NONE = 0

Public Const HKEY_CURRENT_USER = &H80000001         ' Reference a section of the registry.
Public Const HKEY_LOCAL_MACHINE = &H80000002
Public Const HKEY_CLASSES_ROOT = &H80000000

Public Const KEY_ALL_ACCESS = &H3F                  ' This is a simplified version of the declaration in WINCEAPI.TXT
Public Const REG_DWORD = 4                          ' 32-bit number.
Public Const REG_OPTION_NON_VOLATILE = 0            ' Key is preserved when system is rebooted.
Public Const REG_SZ = 1                             ' Unicode nul terminated string.

Declare Function RegCloseKey Lib "Coredll" ( _
  ByVal hKey As Long) _
  As Long

Declare Function RegCreateKeyEx Lib "Coredll" Alias "RegCreateKeyExW" ( _
  ByVal hKey As Long, _
  ByVal lpSubKey As String, _
  ByVal Reserved As Long, _
  ByVal lpClass As String, _
  ByVal dwOptions As Long, _
  ByVal samDesired As Long, _
  ByVal lpSecurityAttributes As Long, _
  phkResult As Long, _
  lpdwDisposition As Long) _
  As Long

Declare Function RegDeleteKey Lib "Coredll" Alias "RegDeleteKeyW" ( _
  ByVal hKey As Long, _
  ByVal lpSubKey As String) _
  As Long

Declare Function RegOpenKeyEx Lib "Coredll" Alias "RegOpenKeyExW" ( _
  ByVal hKey As Long, _
  ByVal lpSubKey As String, _
  ByVal ulOptions As Long, _
  ByVal samDesired As Long, _
  phkResult As Long) _
  As Long

Declare Function RegQueryValueExLong Lib "Coredll" Alias "RegQueryValueExW" ( _
  ByVal hKey As Long, _
  ByVal lpValueName As String, _
  ByVal lpReserved As Long, _
  lpType As Long, _
  lpData As Long, _
  lpcbData As Long) As Long

Declare Function RegQueryValueEx Lib "Coredll" Alias "RegQueryValueExW" ( _
  ByVal hKey As Long, _
  ByVal lpValueName As String, _
  ByVal lpReserved As Long, _
  lpType As Long, _
  ByVal lpData As Long, _
  lpcbData As Long) _
  As Long

Declare Function RegQueryValueExString Lib "Coredll" Alias "RegQueryValueExW" ( _
  ByVal hKey As Long, _
  ByVal lpValueName As String, _
  ByVal lpReserved As Long, _
  lpType As Long, _
  ByVal lpData As String, _
  lpcbData As Long) _
  As Long

Declare Function RegSetValueExLong Lib "Coredll" Alias "RegSetValueExW" ( _
  ByVal hKey As Long, _
  ByVal lpValueName As String, _
  ByVal Reserved As Long, _
  ByVal dwType As Long, _
  lpValue As Long, _
  ByVal cbData As Long) _
  As Long

Declare Function RegSetValueExString Lib "Coredll" Alias "RegSetValueExW" ( _
  ByVal hKey As Long, _
  ByVal lpValueName As String, _
  ByVal Reserved As Long, _
  ByVal dwType As Long, _
  ByVal lpValue As String, _
  ByVal cbData As Long) _
  As Long
  
Public Function CreateNewKey(lSection As Long, _
                             sNewKeyName As String)
       
Dim hNewKey As Long         '-- Handle to the new key
Dim lRetVal As Long         '-- Result of the RegCreateKeyEx function

'-- Create Registry Key
'-- If key already exists, nothing happens
lRetVal = RegCreateKeyEx(lSection, sNewKeyName, CLng(0), _
          vbNullString, REG_OPTION_NON_VOLATILE, _
          KEY_ALL_ACCESS, _
          CLng(0), hNewKey, lRetVal)

'-- Return Handle to Key
CreateNewKey = hNewKey

'-- Close Registry Handle
RegCloseKey (hNewKey)

End Function
Public Function QueryValue(lSection As Long, _
                           sKeyName As String, _
                           sValueName As String)

       Dim lRetVal  As Long         'result of the API functions
       Dim hKey     As Long         'handle of opened key
       Dim vValue   As Variant      'setting of queried value

       '-- Open Registry Key
       lRetVal = RegOpenKeyEx(lSection, sKeyName, 0, _
                KEY_ALL_ACCESS, hKey)
       
       '-- Query Registry Value
       lRetVal = QueryValueEx(hKey, sValueName, vValue)
       
       '-- Return Value
       QueryValue = vValue
       
       '-- Close Reg Key
       RegCloseKey (hKey)

End Function
'SetValueEx and QueryValueEx Wrapper Functions:
Public Function SetValueEx(ByVal hKey As Long, _
                           sValueName As String, _
                           lType As Long, vValue As Variant) As Long
       
    Dim lValue As Long
    Dim sValue As String
    
    Select Case lType
        Case REG_SZ
            sValue = vValue & Chr(0)
            SetValueEx = RegSetValueExString(hKey, sValueName, CLng(0), _
                                           lType, CStr(sValue), Len(sValue) * 2)
        Case REG_DWORD
            lValue = vValue
            SetValueEx = RegSetValueExLong(hKey, sValueName, CLng(0), _
                                           lType, CLng(lValue), 4)
    End Select
    
End Function
Function QueryValueEx(ByVal lhKey As Long, _
                      ByVal szValueName As String, _
                      vValue As Variant) As Long
       
    Dim cch      As Long
    Dim lrc      As Long
    Dim lType    As Long
    Dim lValue   As Long
    Dim sValue   As String

    '-- Determine the size and type of data to be read
    lrc = RegQueryValueEx(lhKey, szValueName, CLng(0), lType, CLng(0), cch)
    
    Select Case lType
        '-- For strings [n.b. this is unicode so one char = 2 bytes]
        Case REG_SZ:
            sValue = String(cch / 2, 0)
            lrc = RegQueryValueExString(lhKey, szValueName, CLng(0), lType, _
                                        sValue, cch)
            If lrc = ERROR_NONE Then
                vValue = Left(sValue, (cch / 2) - 1)
            Else
                vValue = Empty
            End If
        
        '-- For DWORDS
        Case REG_DWORD:
            lrc = RegQueryValueExLong(lhKey, szValueName, CLng(0), lType, _
                                      lValue, cch)
            If lrc = ERROR_NONE Then vValue = lValue
        Case Else
            '-- All other data types not supported
            lrc = -1
    End Select

End Function
Public Sub SetKeyValue(lSection As Long, _
                       sKeyName As String, _
                       sValueName As String, _
                       vValueSetting As Variant, _
                       lValueType As Long)
   
    Dim lRetVal  As Long         '-- Result of the SetValueEx function
    Dim hKey     As Long         '-- Handle of open key
       
    '-- Open the specified key
    lRetVal = RegOpenKeyEx(lSection, sKeyName, _
                           0, _
                           KEY_ALL_ACCESS, hKey)
                              
    '-- Set New Value
    lRetVal = SetValueEx(hKey, sValueName, lValueType, vValueSetting)
    
    '-- Close Reg Key
    RegCloseKey (hKey)
End Sub

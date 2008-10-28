Attribute VB_Name = "mSettings"
Option Explicit

Private m_url As String
Private m_user As String
Private m_pass As String
Private m_blogid As String
Private m_footer As String
Private m_loaded As Boolean

Private Const PB_SETTINGS_KEY = "Software\SimonFell\PocketBlogger"

Public Function getUrl()
    ensureLoaded
    getUrl = m_url
End Function

Public Function getUser()
    ensureLoaded
    getUser = m_user
End Function

Public Function getPass()
    ensureLoaded
    getPass = m_pass
End Function

Public Function getBlogId()
    ensureLoaded
    getBlogId = m_blogid
End Function

Public Function getFooter()
    ensureLoaded
    getFooter = m_footer
End Function

Public Sub saveSettings(ByVal url, ByVal user, ByVal pass, ByVal blogId, ByVal footer)
    m_url = url
    m_user = user
    m_pass = pass
    m_blogid = blogId
    m_footer = footer
    SetKeyValue HKEY_CURRENT_USER, PB_SETTINGS_KEY, "URL", url, REG_SZ
    SetKeyValue HKEY_CURRENT_USER, PB_SETTINGS_KEY, "USER", user, REG_SZ
    SetKeyValue HKEY_CURRENT_USER, PB_SETTINGS_KEY, "PASS", pass, REG_SZ
    SetKeyValue HKEY_CURRENT_USER, PB_SETTINGS_KEY, "BLOGID", blogId, REG_SZ
    SetKeyValue HKEY_CURRENT_USER, PB_SETTINGS_KEY, "FOOTER", footer, REG_SZ
End Sub

Private Sub ensureLoaded()
    If m_loaded Then Exit Sub
    Dim hKey, lRes
    lRes = RegOpenKeyEx(HKEY_CURRENT_USER, PB_SETTINGS_KEY, 0, KEY_ALL_ACCESS, hKey)
    If (lRes <> 0) Then
        CreateNewKey HKEY_CURRENT_USER, PB_SETTINGS_KEY
        m_footer = "<div style='text-align:right;font-size:0.8em;'>[powered by PocketBlogger]</div>"
    Else
        m_url = "" & QueryValue(HKEY_CURRENT_USER, PB_SETTINGS_KEY, "URL")
        m_user = "" & QueryValue(HKEY_CURRENT_USER, PB_SETTINGS_KEY, "USER")
        m_pass = "" & QueryValue(HKEY_CURRENT_USER, PB_SETTINGS_KEY, "PASS")
        m_blogid = "" & QueryValue(HKEY_CURRENT_USER, PB_SETTINGS_KEY, "BLOGID")
        m_footer = "" & QueryValue(HKEY_CURRENT_USER, PB_SETTINGS_KEY, "FOOTER")
        RegCloseKey (hKey)
    End If
    m_loaded = True
End Sub

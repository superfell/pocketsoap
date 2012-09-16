dim h, r, s
set h = CreateObject("Pocket.HTTP")
h.method = "POST"
s = "the quick brown fox jumped over the lazy dog"

set r = h.getResponse("http://localhost/ph-tests/echo.aspx", s)

res = r.String

if res <> s then
	wscript.echo "got wrong response, expecting " & s & " but got " & res
else
	wscript.echo "passed"
end if
@setlocal
@set ToolName=%~n0%
@%PYTHON3% %BASE_TOOLS_PATH%\Source\Python\%ToolName%\%ToolName%.py %*

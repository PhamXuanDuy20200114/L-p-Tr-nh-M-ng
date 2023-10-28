Nhập user name:
+ Nếu có user name: Insert password (Không có thì Account is not active)
-> đúng (Login success)
-> sai (Insert password thêm 2 lần) -> nếu vẫn sai block 
                                    -> nếu đúng login success
+ Nếu login success:
-> nhập sâu mật khẩu mới hoặc log out bằng "bye" 
    -> Nếu nhập xâu sai (có ký tự đặc biệt) -> Error String
    -> Nếu nhập xâu đúng -> gửi cho client hash string
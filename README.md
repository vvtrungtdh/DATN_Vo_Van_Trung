Dự án Thiết kế hệ thống chẩn đoán lỗi và cập nhật chương trình trên ECUs ô tô.
Dự án này được thực hiện tại công ty TNHH FPT software Miền Trung. Nhóm sinh viên thực hiện 
gồm 2 thành viên: Võ Văn Trung và Hà Minh Hiếu thực hiện
cho DATN tại trường đại học Bách khoa Đà Nẵng vào năm 2023-2024.
Thời gian thực hiện dự án từ tháng 8 năm 2023 đến tháng 12 năm 2023. 
Mô tả:
Dự án yêu cầu 5 thành phần chính bao gồm:
 - xây dựng một phần mềm trên máy tính đóng vai trò công cụ chẩn đoán tương tác với người dùng,
   nhằm hỗ trợ họ trong việc chẩn đoán lỗi trên ô tô và cập nhật chương trình một cách nhanh chóng. Công cụ được xây dựng để kết nối với thiết bị chẩn đoán (S32K144 EVB của NXP)
   truyền nhận dữ liệu thông qua giao thức UART. Các chức năng của công cụ chẩn đoán này được viết dựa trên mô tả đối với Clients trong tiêu chuẩn ISO - 14229-1 (UDS) về cung cấp
   các dịch vụ chẩn đoán và cập nhật chương trình trên xe ô tô. Các dịch vụ sẽ được hiển thị trên giao diện màn hình máy tính để người dùng lựa chọn. Ngoài ra còn tích hợp một số
   tính năng về hướng dẫn sử dụng cũng được hỗ trợ nhằm giúp cho người dùng dễ dàng sử dụng.
- xây dựng chương trình cập nhật phần mềm bằng Bootloader trên ECUs. Nhóm thực hiện việc tìm hiểu về cách thức khởi động của MCU, quá trình biên dịch chương trình, bộ nhớ trong vi
  điều khiển... Từ đó nắm được cách thức để đưa MCU nhảy đến chương trình đã được nạp vào bộ nhớ flash.  Quy trình hoạt động tối ưu nhằm giúp cho quá trình cập nhật trở nên an toàn
  hơn nếu như có lỗi xảy ra. 
- Xây dựng một bộ driver CAN-TP dựa trên nền tảng là giao thức CAN thường và tiêu chuẩn ISO-15765-2 về CANTP. Tối ưu hóa code và cung cấp các hàm giúp truyền một lượng lớn dữ liệu
  (bao gồm file hex chương trình cần cập nhật) giữa 2 thiết bị ECUs và thiết bị chẩn đoán.
- Xây dựng bộ giao thức UDS trên chương trình hiện hành ECUs, chương trình bootloader ECU và công cụ chẩn đoán PC tool. Thuật toán cho bộ giao thức sẽ cung cấp các dịch vụ về bảo mật,
  cập nhật chương trình và chẩn đoán lỗi cho từng vị trí khác nhau.
- Xây dựng thuật toán chẩn đoán lỗi DTC được thực hiện theo mô tả trong tiêu chuẩn ISO-14229-1 về mã lỗi chẩn đoán DTC, ảnh chụp nhanh dữ liệu DTC, 8 bit mô tả trạng thái lỗi đang xảy
  ra và lưu vào bộ nhớ dài hạn của ECU khi phát hiện lỗi.
Kết luận:
Dự án thực hiện thành công mục tiêu ban đầu, đáp ứng được các yêu cầu về mặt kỹ thuật và quy tắc được đặt ra. Sản phẩm hoạt động tốt sau nhiều lần thử nghiệm. 

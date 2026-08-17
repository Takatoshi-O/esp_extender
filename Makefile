# .cファイルの実体がある場所（vpathがファイル名だけで検索する）
KERNEL_DIRS  += $(SPIKE_RT_DIR)/mylib/src

# ビルド・リンク対象のオブジェクトファイル
SYSSVC_COBJS += lump_comm.o lump_receive.o lump_color.o lump_camera.o lump_dispatch.o lump_camera_calib.o lump_sensors.o

# ヘッダ検索パスはincludeのルートだけ通す
INCLUDES += -I$(SPIKE_RT_DIR)/mylib/include \
			-I$(SPIKE_RT_DIR)/mylib
vp_model(NAME i2c_corruptor
    PREFIX "devices/i2c/corruptor"
    SOURCES "i2c_corruptor.cpp"
    )

target_link_libraries(i2c_corruptor PRIVATE i2c_helper)

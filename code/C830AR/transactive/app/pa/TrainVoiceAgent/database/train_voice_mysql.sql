-- Apply through the project's database deployment process. These tables do not use AQ.
CREATE TABLE train_voice_train (
    train_id SMALLINT UNSIGNED NOT NULL PRIMARY KEY,
    train_name VARCHAR(128) NOT NULL,
    enabled TINYINT(1) NOT NULL DEFAULT 1
) ENGINE=InnoDB;

CREATE TABLE train_voice_car (
    car_id INT UNSIGNED NOT NULL PRIMARY KEY,
    train_id SMALLINT UNSIGNED NOT NULL,
    car_name VARCHAR(128) NOT NULL,
    car_position SMALLINT UNSIGNED NOT NULL,
    enabled TINYINT(1) NOT NULL DEFAULT 1,
    UNIQUE KEY uq_tv_car_position (train_id, car_position),
    UNIQUE KEY uq_tv_train_car (train_id, car_id),
    CONSTRAINT fk_tv_car_train FOREIGN KEY (train_id) REFERENCES train_voice_train(train_id)
) ENGINE=InnoDB;

CREATE TABLE train_voice_device (
    device_key INT UNSIGNED NOT NULL PRIMARY KEY,
    car_id INT UNSIGNED NOT NULL,
    device_type VARCHAR(32) NOT NULL,
    device_name VARCHAR(128) NOT NULL,
    protocol_device_id SMALLINT UNSIGNED NOT NULL,
    ip_address VARCHAR(45) NOT NULL DEFAULT '',
    control_port SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    sip_extension VARCHAR(64) NULL,
    enabled TINYINT(1) NOT NULL DEFAULT 1,
    UNIQUE KEY uq_tv_sip (sip_extension),
    UNIQUE KEY uq_tv_car_device (car_id, device_type, protocol_device_id),
    UNIQUE KEY uq_tv_car_device_key (car_id, device_key),
    CONSTRAINT fk_tv_device_car FOREIGN KEY (car_id) REFERENCES train_voice_car(car_id)
) ENGINE=InnoDB;

CREATE TABLE train_voice_active_call (
    call_id VARCHAR(128) NOT NULL PRIMARY KEY,
    train_id SMALLINT UNSIGNED NOT NULL,
    car_id INT UNSIGNED NOT NULL,
    device_key INT UNSIGNED NOT NULL,
    audio_function SMALLINT UNSIGNED NOT NULL,
    sip_extension VARCHAR(64) NOT NULL,
    sip_call_id VARCHAR(255) NOT NULL,
    client_id VARCHAR(128) NOT NULL,
    started_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    KEY ix_tv_call_train_function (train_id, audio_function),
    CONSTRAINT fk_tv_call_train_car FOREIGN KEY (train_id, car_id) REFERENCES train_voice_car(train_id, car_id),
    CONSTRAINT fk_tv_call_car_device FOREIGN KEY (car_id, device_key) REFERENCES train_voice_device(car_id, device_key)
) ENGINE=InnoDB;

-- This script only defines the table; deployment must apply it separately.
CREATE TABLE train_voice_call_request (
    request_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    train_id SMALLINT UNSIGNED NOT NULL,
    request_type VARCHAR(16) NOT NULL,
    source_device_id SMALLINT UNSIGNED NOT NULL,
    received_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_tv_pending_request (train_id, request_type, source_device_id)
) ENGINE=InnoDB;

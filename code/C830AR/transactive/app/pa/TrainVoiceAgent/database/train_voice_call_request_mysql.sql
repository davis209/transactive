-- For deployments that already created the other train_voice tables.
CREATE TABLE train_voice_call_request (
    request_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    train_id SMALLINT UNSIGNED NOT NULL,
    request_type VARCHAR(16) NOT NULL,
    source_device_id SMALLINT UNSIGNED NOT NULL,
    received_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_tv_pending_request (train_id, request_type, source_device_id)
) ENGINE=InnoDB;

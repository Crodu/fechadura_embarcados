import { Button, Link, Modal, Stack, TextField, Typography } from "@mui/material"
import NextLink from 'next/link';
import PropTypes from 'prop-types';
import * as Yup from 'yup';
import { Box } from "@mui/system"
import { useFormik } from "formik";
import { BACKEND_URL } from "src/contexts/api";

const style = {
    position: 'absolute',
    top: '50%',
    left: '50%',
    transform: 'translate(-50%, -50%)',
    width: 400,
    bgcolor: 'background.paper',
    border: '2px solid #000',
    boxShadow: 24,
    p: 4,
  };

export const ModalAdd = ({open, setOpen, props}) => {

    const handleClose = () => {
        setOpen(false);
    };

    const formik = useFormik({
        initialValues: {
          rfid_tag: '',
          username: '',
          password: '',
          submit: null
        },
        validationSchema: Yup.object({
          username: Yup
            .string()
            .max(255)
            .required('Name is required'),
          password: Yup
            .string()
            .max(255)
            .required('Password is required')
        }),
        onSubmit: async (values, helpers) => {
          try {
            const response = await fetch(`${BACKEND_URL}/users/`, {
                method: 'POST',
                headers: {
                  'Content-Type': 'application/json'
                },
                body: JSON.stringify({ username: values.username, password: values.password, rfid_tag: values.rfid_tag})
              });
          } catch (err) {
            helpers.setStatus({ success: false });
            helpers.setErrors({ submit: err.message });
            helpers.setSubmitting(false);
          }
        }
      });
    

    return (
        <Modal
            open={open}
            onClose={handleClose}
            aria-labelledby="modal-modal-title"
            aria-describedby="modal-modal-description"
            >
            <Box sx={style}>
                <Box
                    sx={{
                    flex: '1 1 auto',
                    alignItems: 'center',
                    display: 'flex',
                    justifyContent: 'center'
                    }}
                >
                    <Box
                    sx={{
                        maxWidth: 550,
                        px: 3,
                        py: '100px',
                        width: '100%'
                    }}
                    >
                    <div>
                        <Stack
                        spacing={1}
                        sx={{ mb: 3 }}
                        >
                        <Typography variant="h4">
                            New User
                        </Typography>
                        </Stack>
                        <form
                        noValidate
                        onSubmit={formik.handleSubmit}
                        >
                        <Stack spacing={3}>
                            <TextField
                            error={!!(formik.touched.username && formik.errors.username)}
                            fullWidth
                            helperText={formik.touched.username && formik.errors.username}
                            label="Name"
                            name="username"
                            onBlur={formik.handleBlur}
                            onChange={formik.handleChange}
                            value={formik.values.username}
                            />
                            <TextField
                            error={!!(formik.touched.password && formik.errors.password)}
                            fullWidth
                            helperText={formik.touched.password && formik.errors.password}
                            label="Password"
                            name="password"
                            onBlur={formik.handleBlur}
                            onChange={formik.handleChange}
                            type="number"
                            value={formik.values.password}
                            />
                            <TextField
                            error={!!(formik.touched.rfid_tag && formik.errors.rfid_tag)}
                            fullWidth
                            helperText={formik.touched.rfid_tag && formik.errors.rfid_tag}
                            label="RFID Tag"
                            name="rfid_tag"
                            onBlur={formik.handleBlur}
                            onChange={formik.handleChange}
                            value={formik.values.rfid_tag}
                            />
                            
                        </Stack>
                        {formik.errors.submit && (
                            <Typography
                            color="error"
                            sx={{ mt: 3 }}
                            variant="body2"
                            >
                            {formik.errors.submit}
                            </Typography>
                        )}
                        <Button
                            fullWidth
                            size="large"
                            sx={{ mt: 3 }}
                            type="submit"
                            variant="contained"
                        >
                            Continue
                        </Button>
                        </form>
                    </div>
                    </Box>
                </Box>
            </Box>
        </Modal>
    )
}

ModalAdd.propTypes = {
    open: PropTypes.bool,
    setOpen: PropTypes.func,
}

ModalAdd.defaultProps = {
    open: false,
    setOpen: () => {},
}
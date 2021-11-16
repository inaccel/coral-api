from setuptools import setup

setup(
    name='coral-api',
    description='Enterprise-Grade Accelerator Orchestration',
    author='InAccel',
    author_email='info@inaccel.com',
    url='https://inaccel.com',
    packages=[
        'inaccel.coral',
    ],
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'License :: OSI Approved :: Apache Software License',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
    ],
    license='Apache-2.0',
    keywords=[
        'InAccel',
        'Coral',
        'API',
    ],
    platforms=[
        'Linux',
    ],
    package_dir={
        '': 'src/main/python',
    },
    package_data={
        '': [
            'native/lib*.so',
        ],
    },
    install_requires=[
        'numpy-allocator',
    ],
    python_requires='>=3.8',
    namespace_packages=[
        'inaccel',
    ],
)
